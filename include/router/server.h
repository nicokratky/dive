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

#ifndef DIVE_ROUTER_SERVER_H
#define DIVE_ROUTER_SERVER_H

#include <cstdint>
#include <string>
#include <memory>

#include "asio.hpp"
#include "spdlog/spdlog.h"

#include "dive.pb.h"


/*!
 * \brief TCP server, can only receive messages
 */
class Server {
  public:
    /*!
     * \brief Constructor
     *
     * \param io_context    Reference to asio::io_context object
     * \param port          Port to which the server will bind to
     */
    Server(asio::io_context& io_context, unsigned short port);

    ~Server();

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

#endif  // DIVE_ROUTER_SERVER_h
