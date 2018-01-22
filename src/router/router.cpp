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

using namespace asio::ip;

Router::Router(const std::string& router_id, const std::string& ip_address,
               unsigned short port, asio::io_context& io_context) :
    router_id_{router_id},
    ip_address_{ip_address},
    port_{port},
    io_context_{io_context},
    endpoint_{tcp::v4(), port_},
    acceptor_{io_context_, endpoint_},
    resolver_{io_context_} {
}


Router::Router(const std::string& router_id, const std::string& ip_address,
               unsigned short port, asio::io_context& io_context,
               std::shared_ptr<spdlog::logger> logger) :
    Router(router_id, ip_address, port, io_context) {
    logger_ = logger;
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

    send_update();
}


void Router::send_update() {
    logger_->info("Updating neighbours");

    dive::DistanceVector dv{pack_distance_vector()};

    asio::error_code ec;

    auto node{distance_vector_.begin()};
    while (node != distance_vector_.end()) {
        if (node->second == 1) {
            // node is neighbour
            logger_->debug("Sending update to {}", node->first);

            tcp::resolver::results_type endpoint{resolver_.resolve(
                    links_[node->first].ip_address,
                    std::to_string(links_[node->first].port))};

            tcp::socket sock{io_context_};

            asio::connect(sock, endpoint, ec);

            if (ec) {
                logger_->error("Connecting to {} failed. Retrying",
                               node->first);
                std::this_thread::sleep_for(std::chrono::seconds(1));
                continue;
            }

            logger_->debug("Connected to {}", node->first);

            asio::streambuf b;
            std::ostream os{&b};

            dv.SerializeToOstream(&os);

            uint32_t length{htonl(b.size())};

            asio::write(sock, asio::buffer(&length, sizeof(length)));
            asio::write(sock, b);

            logger_->debug("Distance Vector sent");

            sock.close();
        }

        ++node;
    }
}


void Router::receive_updates() {
    logger_->info("Starting listening for updates");
    acceptor_.listen();

    for (;;) {
        tcp::socket sock{io_context_};
        acceptor_.accept(sock);

        logger_->debug("Got connection");

        uint32_t length;

        std::size_t bytes_read = asio::read(sock,
                                            asio::buffer(&length,
                                                         sizeof(length)),
                                            asio::transfer_exactly(4));

        length = ntohl(length);

        dive::DistanceVector dv;

        asio::streambuf b;
        asio::streambuf::mutable_buffers_type buf{b.prepare(length)};

        bytes_read = read(sock, buf);

        b.commit(bytes_read);

        std::istream is{&b};
        dv.ParseFromIstream(&is);

        sock.close();
    }
}


dive::DistanceVector Router::pack_distance_vector() {
    dive::DistanceVector dv;

    dv.set_router_id(router_id_);

    for (const auto& node: distance_vector_) {
        dive::DistanceVector::Distance* d{dv.add_distance_vector()};

        d->set_router_id(node.first);
        d->set_distance(node.second);
    }

    return dv;
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
