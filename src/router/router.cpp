/*!
 * \author Nico Kratk
 * \file router.cpp
 * \copyright Copyright 2018 Nico Kratky.
 * This project is released under the Boost Software License.
 *
 */

#include "router.h"

#include <iostream>
#include <string>

#include "fmt/format.h"

#include "json.hpp"
using json = nlohmann::json;

Router::Router(std::string router_id) :
    router_id_{router_id} {
}


void Router::set_cost(std::string& router_id, int cost) {
    distance_vector_[router_id] = cost;
}


void Router::initialize_from_json(json nodes, json links) {
    for (json::iterator it = nodes.begin(); it != nodes.end(); ++it) {
        if (it.key() == router_id_) {
            distance_vector_[router_id_] = 0;
        } else {
            distance_vector_[it.key()] = -1;
        }
    }

    for (const auto& link : links) {
        if (link["source"] == router_id_) {
            // link is a neighbour
            distance_vector_[link["target"].get<std::string>()] = 1;
        } else if (link["target"] == router_id_) {
            // link is a neighbour
            distance_vector_[link["source"].get<std::string>()] = 1;
        }
    }
}


std::ostream& operator<<(std::ostream& os, const Router& r) {
    for (const auto& node : r.distance_vector_) {
        os << fmt::format("{:<5}", node.first);
    }
    os << std::endl;

    for (const auto& node : r.distance_vector_) {
        os << fmt::format("{:<5}", node.second);
    }
    os << std::endl;

    return os;
}
