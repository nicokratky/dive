/*!
 * \author Nico Kratky
 * \file client.cpp
 * \copyright Copyright 2018 Nico Kratky.
 * This project is released under the Boost Software License.
 *
 * matnr: i13083
 * catnr: 13
 * class: 5cHIF
 */

#include "client.h"

#include <cstdint>
#include <string>

#include "asio.hpp"
#include "spdlog/spdlog.h"

using asio::ip::tcp;

Client::Client(asio::io_context& io_context) :
    logger_{spdlog::stdout_color_mt("Client")},
    io_context_{io_context},
    resolver_{io_context}
{}


std::size_t Client::send_to(const std::string& ip_address, unsigned short port,
                            const std::string& message) {
    logger_->debug("Sending message to {}:{}", ip_address, port);

    tcp::resolver::results_type endpoints{
        resolver_.resolve(ip_address, std::to_string(port))};

    tcp::socket sock{io_context_};
    connect(sock, endpoints);

    uint32_t prefix{make_prefix(message)};

    logger_->trace("Message prefix: {}", prefix);

    asio::error_code ec;

    std::size_t sent{0};

    sent += asio::write(sock, asio::buffer(&prefix, sizeof(prefix)), ec);
    sent += asio::write(sock, asio::buffer(message), ec);

    logger_->info("Message sent");

    sock.close();

    return sent;
}


void Client::connect(asio::ip::tcp::socket& sock,
                     asio::ip::tcp::resolver::results_type& endpoints) {
    bool connected{false};
    asio::error_code ec;

    while (!connected) {
        asio::connect(sock, endpoints, ec);

        if (!ec) {
            connected = true;
        } else if (ec != asio::error::connection_refused) {
            logger_->error("Error while connecting: {}", ec.message());
        }
    }
}


uint32_t Client::make_prefix(const std::string& message) {
    std::size_t length{message.size()};
    uint32_t nlength{htonl(length)};

    return nlength;
}

