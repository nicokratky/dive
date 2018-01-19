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

/*
 * Vendor header files
 */
#include "spdlog/spdlog.h"

#include "fmt/format.h"

#include "json.hpp"
using json = nlohmann::json;

Router::Router(std::string& router_id, std::string& ip_address, int port) :
    router_id_{router_id},
    ip_address_{ip_address},
    port_{port} {
}


Router::Router(std::string& router_id, std::string& ip_address, int port,
               std::shared_ptr<spdlog::logger> logger) :
    Router(router_id, ip_address, port) {
    logger_ = logger;
}


void Router::set_cost(std::string& router_id, int cost) {
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
            int port{nodes[it.key()]["port"].get<int>()};

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
