/*
 * name: Nico Kratky
 * matnr: i13083
 * catnr: 13
 * class 5cHIF
 * file: main.cpp
 */

#include <iostream>
#include <fstream>

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

int main(int argc, char** argv) {
    std::string input;
    std::string id;

    auto cli = (
        clipp::value("topology file", input),
        clipp::value("router id", id)
    );

    if (!clipp::parse(argc, argv, cli)) {
        std::cout << clipp::make_man_page(cli, argv[0]);
    }

    fmt::print("Path to topology file: {}\n", input);
    fmt::print("Router ID: {}\n", id);

    std::ifstream f{input};
    json j;
    f >> j;

    std::cout << j << std::endl;
}
