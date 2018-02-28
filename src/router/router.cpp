/*!
 * \author Nico Kratky
 * \file router.cpp
 * \copyright Copyright 2018 Nico Kratky.
 * This project is released under the Boost Software License.
 *
 */

/*
 * header file related to this file
 */
#include "router.h"

#include <iostream>
#include <string>
#include <memory>
#include <thread>
#include <chrono>
#include <mutex>
#include <random>

#include "asio.hpp"
#include "spdlog/spdlog.h"
#include "fmt/format.h"
#include "nlohmann/json.hpp"

#include "dive.pb.h"
#include "server.h"
#include "client.h"

using json = nlohmann::json;
using asio::ip::tcp;

Router::Router(const std::string& router_id, const std::string& ip_address,
               unsigned short port, asio::io_context& io_context,
               std::chrono::seconds interval) :
    logger_{spdlog::stdout_color_mt("Router")},
    router_id_{router_id},
    ip_address_{ip_address},
    port_{port},
    io_context_{io_context},
    server_{io_context_, port_},
    client_{io_context_},
    interval_{interval} {
}


void Router::initialize_from_json(json nodes, json links) {
    // Loop through all nodes and store there connection informations
    for (json::iterator it = nodes.begin(); it != nodes.end(); ++it) {
        if (it.key() == router_id_) {
            distance_vector_[router_id_] = 0;
        } else {
            distance_vector_[it.key()] = kInfinity;

            std::string ip_address{
                nodes[it.key()]["ip_address"].get<std::string>()};
            unsigned short port{nodes[it.key()]["port"].get<unsigned short>()};

            Link l{ip_address, port};
            links_[it.key()] = l;
        }
    }

    // Loop through all links and store neighbours
    for (const auto& link : links) {
        std::string target;
        if (link["source"] == router_id_) {
            // link is a neighbour
            target = link["target"].get<std::string>();

            distance_vector_[target] = 1;

            links_[target].next_hop = target;

            if (link.find("pof") != link.end()) {
                links_[target].pof = link["pof"];
            }
        } else if (link["target"] == router_id_) {
            // link is a neighbour
            target = link["source"].get<std::string>();

            distance_vector_[target] = 1;

            links_[target].next_hop = target;
        }
    }
}


void Router::run() {
    std::thread update_thread{&Router::receive_updates, this};
    update_thread.detach();

    std::thread outage_thread{&Router::simulate_outage, this};
    outage_thread.detach();

    for (int i{0};; ++i) {
        std::cout << fmt::format("Distance vector after {} iterations:", i)
                  << std::endl;
        std::cout << *this << std::endl;

        update_neighbours();
        std::this_thread::sleep_for(interval_);
    }
}


void Router::update_neighbours() {
    std::string distance_vector_update{pack_distance_vector()};

    for (const auto& node : distance_vector_) {
        // only update node if it is a direct neighbour
        // link has to be up
        if (node.second != 1 || !links_[node.first].up)
            continue;

        logger_->info("Sending update to {}", node.first);

        client_.send_to(links_[node.first].ip_address,
                        links_[node.first].port,
                        distance_vector_update);
    }
}


void Router::receive_updates() {
    for (;;) {
        logger_->info("Waiting for update");
        dive::Message update{server_.receive()};

        if (update.has_dv()) {
            update_distance_vector(update.dv());
        } else if (update.has_cm()) {
            handle_control_message(update.cm());
        }
    }
}


std::string Router::pack_distance_vector() {
    dive::Message message;
    dive::DistanceVector* dv{message.mutable_dv()};

    dv->set_router_id(router_id_);

    {
        std::lock_guard<std::mutex> lck{dv_mtx_};

        for (const auto& node : distance_vector_) {
            dive::DistanceVector::Distance* d{dv->add_distance_vector()};

            d->set_router_id(node.first);
            d->set_distance(node.second);
        }
    }

    std::string container;

    message.SerializeToString(&container);

    return container;
}


std::string Router::pack_control_message(const std::string& data) {
    dive::Message message;
    dive::ControlMessage* cm{message.mutable_cm()};

    cm->set_router_id(router_id_);
    cm->set_data(data);

    std::string container;

    message.SerializeToString(&container);

    return container;
}


void Router::update_distance_vector(dive::DistanceVector update) {
    std::string sender{update.router_id()};
    logger_->info("Got update from {}", sender);

    std::lock_guard<std::mutex> lck{dv_mtx_};

    for (const auto& node : update.distance_vector()) {
        std::string node_id{node.router_id()};
        if (node_id != sender && node_id != router_id_) {
            logger_->trace("Processing node {} in update from {}", node_id,
                                                                   sender);

            if (node.distance() > 0) {
                int current_cost{distance_vector_[node_id]};
                int new_cost{node.distance() + 1};

                if (current_cost == kInfinity || new_cost < current_cost) {
                    distance_vector_[node_id] = new_cost;
                    links_[node_id].next_hop = sender;
                }
            }
        }
    }
}


void Router::handle_control_message(dive::ControlMessage message) {
    std::string router_id{message.router_id()};
    std::string data{message.data()};

    logger_->info("Got control message from {}: {}", router_id, data);

    if (message.data() == "DOWN") {
        logger_->debug("Link to {} has gone down", router_id);

        {
            // lock both mutexes at the same time
            std::unique_lock<std::mutex> lck1{dv_mtx_, std::defer_lock};
            std::unique_lock<std::mutex> lck2{link_mtx_, std::defer_lock};
            std::lock(lck1, lck2);

            links_[router_id].up = false;
            distance_vector_[router_id] = kInfinity;
        }
    }
}


void Router::simulate_outage() {
    std::this_thread::sleep_for(std::chrono::seconds(20));

    std::random_device rd;
    std::mt19937 gen{rd()};

    for (const auto& link : links_) {
        if (link.second.pof != 0) {
            std::bernoulli_distribution d{link.second.pof};

            if (d(gen)) {
                logger_->debug("Link to {} has gone down", link.first);

                {
                    // lock both mutexes at the same time
                    std::unique_lock<std::mutex> lck1{dv_mtx_,
                                                      std::defer_lock};
                    std::unique_lock<std::mutex> lck2{link_mtx_,
                                                      std::defer_lock};
                    std::lock(lck1, lck2);

                    links_[link.first].up = false;
                    distance_vector_[link.first] = kInfinity;
                }

                client_.send_to(link.second.ip_address,
                                link.second.port,
                                pack_control_message("DOWN"));
            }
        }
    }
}


std::ostream& operator<<(std::ostream& os, const Router& r) {
    os << fmt::format("Router ID: {}\nIP Address: {}\nPort: {}",
                      r.router_id_,
                      r.ip_address_,
                      r.port_) << std::endl;;

    os << fmt::format("+{:-^10}+{:-^10}+\n", "-", "-");
    os << fmt::format("|{:^10}|{:^10}|\n", "Node", "Cost");
    os << fmt::format("+{:-^10}+{:-^10}+\n", "-", "-");

    for (const auto& node : r.distance_vector_) {
        os << fmt::format("|{:^10}|{:^ 10}|\n", node.first, node.second);
    }
    os << fmt::format("+{:-^10}+{:-^10}+\n", "-", "-");

    os << fmt::format("+{:-^10}+{:-^10}+\n", "-", "-");
    os << fmt::format("|{:^10}|{:^10}|\n", "Node", "Next Hop");
    os << fmt::format("+{:-^10}+{:-^10}+\n", "-", "-");

    for (const auto& link : r.links_) {
        os << fmt::format("|{:^10}|{:^10}|\n", link.first,
                                                link.second.next_hop);
    }
    os << fmt::format("+{:-^10}+{:-^10}+\n", "-", "-");


    return os;
}
