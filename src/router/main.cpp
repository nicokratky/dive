/*!
 * \author Nico Kratky
 * \file main.cpp
 * \copyright Copyright 2018 Nico Kratky.
 * This project is released under the Boost Software License.
 *
 * matnr: i13083
 * catnr: 13
 * class: 5cHIF
 */

/*
 * C system files
 */
#include <sys/stat.h>  // stat

/*
 * C++ system files
 */
#include <iostream>
#include <fstream>  // ifstream
#include <map>
#include <string>

/*
 * Vendor header files
 */
#include "asio.hpp"

#include "spdlog/spdlog.h"

#include "fmt/format.h"

#include "json.hpp"
using json = nlohmann::json;

#include "clipp.h"

/*
 * DiVe header files
 */
#include "router.h"

// used by the protobuf example below:
// #include "person.pb.h"

/*!
 * \brief Check if a file exists
 * \param filename path to file
 * \return true if the file exists, else false
 */
bool file_exists(const std::string& filename) {
    struct stat buffer;
    return (stat(filename.c_str(), &buffer) == 0);
}


int main(int argc, char** argv) {
    std::string input;
    std::string id;

    auto cli = (
        clipp::value("topology file", input),
        clipp::value("router id", id));

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
                            nodes.size()) << std::endl;

    std::string ip_address{nodes[id]["ip_address"].get<std::string>()};
    int port{nodes[id]["port"].get<int>()};

    Router router{id, ip_address, port};

    router.initialize_from_json(nodes, links);

    std::cout << router << std::endl;
}
