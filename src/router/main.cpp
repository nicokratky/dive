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

#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <map>
#include <string>

#include "asio.hpp"
#include "spdlog/spdlog.h"
#include "fmt/format.h"
#include "nlohmann/json.hpp"
#include "clipp.h"

#include "router.h"

using json = nlohmann::json;

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
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    std::string input;
    std::string id;
    int interval{5};

    auto cli = (
        clipp::value("topology file", input),
        clipp::value("router id", id),
        clipp::option("-i", "--interval")
            .doc("router update interval (default: " + std::to_string(interval)
                 + ")") & clipp::value("seconds", interval));

    if (!clipp::parse(argc, argv, cli)) {
        std::cout << clipp::make_man_page(cli, argv[0]);
        return 1;
    }

    auto logger = spdlog::stdout_color_mt("logger");
    spdlog::set_level(spdlog::level::trace);

    logger->debug("Topology file: {}", input);
    logger->debug("Router ID: {}", id);

    if (!file_exists(input)) {
        logger->error("{} does not exist.", input);
        return 1;
    }

    std::ifstream f{input};
    json j;

    try {
        f >> j;
    } catch(json::parse_error& e) {
        logger->error("Topology file malformed");
        logger->error("File could not be parsed");
        return 1;
    }

    json nodes{j["nodes"]};
    json links{j["links"]};

    // If either no nodes or links are available, abort
    if (nodes.empty() || links.empty()) {
        logger->error("Topology file malformed");
        logger->error("Nodes or links are missing");
        return 1;
    }

    if (nodes.count(id) != 1) {
        logger->error("{} does not exist or exists multiple times.", id);
        return 1;
    }

    logger->info("Nodes in topology: {}", nodes.size());

    // extract ip_address and port from topology file
    std::string ip_address{nodes[id]["ip_address"].get<std::string>()};
    unsigned short port{nodes[id]["port"].get<unsigned short>()};

    asio::io_context io_context;

    Router router{id, ip_address, port, io_context,
                  std::chrono::seconds(interval)};

    router.initialize_from_json(nodes, links);

    logger->info("Router initialized");

    router.run();

    google::protobuf::ShutdownProtobufLibrary();
}
