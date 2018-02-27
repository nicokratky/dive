/*!
 * \author Nico Kratky
 * \file client.h
 * \copyright Copyright 2018 Nico Kratky.
 * This project is released under the Boost Software License.
 *
 * matnr: i13083
 * catnr: 13
 * class: 5cHIF
 */

#ifndef DIVE_ROUTER_CLIENT_H
#define DIVE_ROUTER_CLIENT_H

#include <cstdint>
#include <string>
#include <memory>

#include "asio.hpp"
#include "spdlog/spdlog.h"

/*!
 * \brief TCP client, can only send messages to a partner
 * Every message will be sent over a seperate connection
 */
class Client {
  public:
    /*!
     * \brief Constructor
     *
     * \param io_context reference to asio::io_context object
     */
    Client(asio::io_context& io_context);

    /*!
     * \brief Send a message
     * Messages are prefixed with its length
     *
     * \param ip_address    Destination IP address
     * \param port          Destination port number
     * \param message       Message to be sent
     *
     * \return Sent bytes
     */
    std::size_t send_to(const std::string& ip_address, unsigned short port,
                        const std::string& message);

  private:
    /*!
     * \brief Connect to an endpoint
     * Method will block until a connection is established
     * If the connection gets refused, try again
     *
     * \param sock      Socket which will be connected
     * \param endpoints Endpoint to which the socket will be connected
     */
    void connect(asio::ip::tcp::socket& sock,
                 asio::ip::tcp::resolver::results_type& endpoints);

    /*!
     * \brief Make the length prefix for a message
     * Prefix is 4 bytes in network byte order (big endian)
     *
     * \param message Message to which the prefix will be generated
     *
     * \return generated prefix
     */
    uint32_t make_prefix(const std::string& message);

    std::shared_ptr<spdlog::logger> logger_;

    asio::io_context& io_context_;
    asio::ip::tcp::resolver resolver_;
};

#endif  // DIVE_ROUTER_CLIENT_H
