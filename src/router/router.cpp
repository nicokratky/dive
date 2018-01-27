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

/*
 * C++ system files
 */
#include <iostream>
#include <string>
#include <memory>
#include <thread>
#include <chrono>

/*
 * Vendor header files
 */
#include "asio.hpp"

#include "spdlog/spdlog.h"

#include "fmt/format.h"

#include "json.hpp"
using json = nlohmann::json;

#include "dive.pb.h"

#include "server.h"
#include "client.h"

using asio::ip::tcp;

Router::Router(const std::string& router_id, const std::string& ip_address,
               unsigned short port, asio::io_context& io_context) :
    logger_{spdlog::stdout_color_mt("Router")},
    router_id_{router_id},
    ip_address_{ip_address},
    port_{port},
    io_context_{io_context},
    server_{io_context_, port_},
    client_{io_context_} {
}


Router::~Router() {
}


void Router::set_cost(const std::string& router_id, int cost) {
    distance_vector_[router_id] = cost;
}


void Router::initialize_from_json(json nodes, json links) {
    // Loop through all nodes and store there connection informations
    for (json::iterator it = nodes.begin(); it != nodes.end(); ++it) {
        if (it.key() == router_id_) {
            distance_vector_[router_id_] = 0;
        } else {
            distance_vector_[it.key()] = -1;

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

            links_[target].next_hop = router_id_;
        } else if (link["target"] == router_id_) {
            // link is a neighbour
            target = link["source"].get<std::string>();

            distance_vector_[target] = 1;

            links_[target].next_hop = router_id_;
        }
    }
}


void Router::run() {
    std::thread update_thread{&Router::receive_updates, this};
    update_thread.detach();

    update_neighbours();
}


void Router::update_neighbours() {
    asio::error_code ec;

    std::string distance_vector_update{pack_distance_vector()};

    for (const auto& node : distance_vector_) {
        // only update node if it is a direct neighbour
        if (node.second != 1)
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
        dive::DistanceVector update{server_.receive()};

        update_distance_vector(update);
    }
}


std::string Router::pack_distance_vector() {
    dive::DistanceVector dv;

    dv.set_router_id(router_id_);

    for (const auto& node: distance_vector_) {
        dive::DistanceVector::Distance* d{dv.add_distance_vector()};

        d->set_router_id(node.first);
        d->set_distance(node.second);
    }

    std::string container;

    dv.SerializeToString(&container);

    return container;
}


void Router::update_distance_vector(dive::DistanceVector update) {
    logger_->info("Got update from {}", update.router_id());
}


std::ostream& operator<<(std::ostream& os, const Router& r) {
    os << fmt::format("Router ID: {}\nIP Address: {}\nPort: {}",
                      r.router_id_,
                      r.ip_address_,
                      r.port_) << std::endl;;

    for (const auto& node : r.distance_vector_) {
        os << fmt::format("{:<5}", node.first);
    }
    os << std::endl;

    for (const auto& node : r.distance_vector_) {
        os << fmt::format("{:<5}", node.second);
    }
    os << std::endl << std::endl;

    for (const auto& link : r.links_) {
        os << fmt::format("{:5}: {}", link.first,
                                      link.second.next_hop) << std::endl;
    }

    return os;
}
