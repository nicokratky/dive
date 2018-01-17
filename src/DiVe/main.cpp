/*!
 * \author Nico Kratky
 * \file main.cpp
 *
 * matnr: i13083
 * catnr: 13
 * class: 5cHIF
 */

#include <iostream>
#include <fstream> // ifstream
#include <sys/stat.h> // stat

// used by the protobuf example below:
// #include <fstream>

// only if using asio
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wconversion"
#include "asio.hpp"
#pragma GCC diagnostic pop

// only if using spdlog
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#include "spdlog/spdlog.h"
#pragma GCC diagnostic pop

// only if using fmt
#include "fmt/format.h"

// only if using json
#include "json.hpp"
using json = nlohmann::json;

// only if using clipp
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#include "clipp.h"
#pragma GCC diagnostic pop

// used by the protobuf example below:
//#include "person.pb.h"

/*!
 * \brief Check if a file exists
 * \param filename path to file
 * \return true if the file exists, else false
 */
bool file_exists(const std::string& filename) {
    struct stat buffer;
    return (stat(filename.c_str(), &buffer) == 0);
}


/*!
 * \brief Print the distance vector
 * \param dv distance vector
 * \return void
 */
void print_distance_vector(std::map<std::string, int>& dv) {
    for (const auto& node: dv) {
        std::cout << fmt::format("{:>8}", node.first);
    }
    std::cout << std::endl;

    for (const auto& node: dv) {
        std::cout << fmt::format("{:>8}", node.second);
    }
    std::cout << std::endl;
}


int main(int argc, char** argv) {
    std::string input;
    std::string id;

    auto cli = (
        clipp::value("topology file", input),
        clipp::value("router id", id)
    );

    if (!clipp::parse(argc, argv, cli)) {
        std::cout << clipp::make_man_page(cli, argv[0]);
        return 1;
    }

    auto logger = spdlog::stdout_color_mt("logger");

    logger->debug("Topology file: {}", input);
    logger->debug("Router ID: {}", id);

    if (!file_exists(input)) {
        logger->error("{} does not exist.", input);
        return 1;
    }

    std::ifstream f{input};
    json j;
    f >> j;

    json nodes = j["nodes"];
    json links = j["links"];

    // If either no nodes or links are available, abort
    if (nodes.empty() || links.empty()) {
        logger->error("Topology file malformed.");
        return 1;
    }

    if (nodes.count(id) != 1) {
        logger->error("{} does not exist or exists multiple times.", id);
        return 1;
    }

    std::cout << fmt::format("Nodes in topology: {}",
                            nodes.size()
                            ) << std::endl;

    std::cout << fmt::format("I am {} ({}, {})",
                            id,
                            nodes[id]["ip_address"].get<std::string>(),
                            nodes[id]["port"].get<int>()
                            ) << std::endl;

    std::map<std::string, int> distance_vector;

    for (json::iterator it = nodes.begin(); it != nodes.end(); ++it) {
        if (it.key() == id) {
            distance_vector[id] = 0;
        }
        else {
            distance_vector[it.key()] = -1;
        }
    }

    for (const auto& link: links) {
        if (link["source"] == id) {
            // link is a neighbour
            distance_vector[link["target"].get<std::string>()] = 1;
        }
        else if (link["target"] == id) {
            // link is a neighbour
            distance_vector[link["source"].get<std::string>()] = 1;
        }
    }

    std::cout << "Initialized Distance Vector" << std::endl;
    print_distance_vector(distance_vector);
}
