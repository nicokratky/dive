/*!
 * \author Nico Kratky
 * \file server.cpp
 * \copyright Copyright 2018 Nico Kratky.
 * This project is released under the Boost Software License.
 *
 * matnr: i13083
 * catnr: 13
 * class: 5cHIF
 */

#include "server.h"

/*
 * C system files
 */
#include <cstdint>

/*
 * C++ system files
 */
#include <string>
#include <thread>
#include <chrono>

/*
 * Vendor header filse
 */
#include "asio.hpp"
#include "spdlog/spdlog.h"

#include "dive.pb.h"

using asio::ip::tcp;

Server::Server(asio::io_context& io_context, unsigned short port) :
    logger_{spdlog::stdout_color_mt("Server")},
    io_context_{io_context},
    acceptor_{io_context_, asio::ip::tcp::endpoint{asio::ip::tcp::v4(), port}}
{}


dive::Message Server::receive() {
    tcp::socket sock{io_context_};
    acceptor_.accept(sock);
    logger_->debug("Somebody connected");

    asio::error_code ec;

    uint32_t length;
    asio::read(sock, asio::buffer(&length, sizeof(length)),
               asio::transfer_exactly(sizeof(length)), ec);

    length = ntohl(length);

    logger_->trace("Got header. Reading message of length {}", length);

    asio::streambuf buffer;
    asio::read(sock, buffer, asio::transfer_exactly(length), ec);

    std::string message{asio::buffers_begin(buffer.data()),
                        asio::buffers_end(buffer.data())};

    logger_->trace("Done reading");

    dive::Message msg;

    if (msg.ParseFromString(message)) {
        logger_->trace("Distance vector parsed successfully");
    } else {
        logger_->error("Distance vector could not be parsed");
    }

    sock.close();

    return msg;
}
