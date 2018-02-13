/*!
 * \author Nico Kratky
 * \file server.h
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
#include <cstdint>

/*
 * C++ system files
 */
#include <string>
#include <memory>

/*
 * Vendor header files
 */
#include "asio.hpp"
#include "spdlog/spdlog.h"

#include "dive.pb.h"

#ifndef ROUTER_SERVER_H
#define ROUTER_SERVER_H

/*!
 * \brief TCP server, can only receive messages
 */
class Server {
  public:
    Server(asio::io_context& io_context, unsigned short port);

    /*!
     * \brief Receive a distance vector from a neighbor
     * Message is parsed to a protobuf object
     *
     * \return parsed protobuf object
     */
    dive::Message receive();

  private:
    std::shared_ptr<spdlog::logger> logger_;

    asio::io_context& io_context_;
    asio::ip::tcp::acceptor acceptor_;
};

#endif  // ROUTER_SERVER_h
