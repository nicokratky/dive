/*!
 * \author Nico Kratky
 * \file router.h
 * \copyright Copyright 2018 Nico Kratky.
 * This project is released under the Boost software License.
 *
 * matnr: i13083
 * catnr: 13
 * class: 5cHIF
 */

#ifndef DIVE_ROUTER_ROUTER_H
#define DIVE_ROUTER_ROUTER_H

/*
 * C++ system files
 */
#include <iostream>
#include <map>
#include <string>
#include <memory>

/*
 * Vendor header files
 */
#include "asio.hpp"

#include "spdlog/spdlog.h"

#include "json.hpp"

#include "dive.pb.h"

/*!
 * \brief Stores link info
 */
struct Link {
    std::string ip_address;
    unsigned short port;

    /*!
     * \brief The router to which a packet has to be sent to reach the final
     *        destination.
     */
    std::string next_hop;

    /*!
     * \brief Default constructor
     */
    Link() {}

    /*!
     * \brief Copy constructor
     */
    Link(const Link& l) :
        ip_address{l.ip_address},
        port{l.port},
        next_hop{l.next_hop} {}

    Link(std::string i, unsigned short p) :
        ip_address{i}, port{p}, next_hop{} {
    }
};

/*!
 * \brief Class to simulate a router
 */
class Router {
  public:
    Router(const std::string& router_id, const std::string& ip_address,
           unsigned short port, asio::io_context& io_context);
    Router(const std::string& router_id, const std::string& ip_address,
           unsigned short port, asio::io_context& io_context,
           std::shared_ptr<spdlog::logger> logger);

    ~Router();

    /*!
    * \brief Set the cost that it takes to reach a router
    *
    * \param router_id
    * \param cost
    *
    * \return void
    */
    void set_cost(const std::string& router_id, int cost);

    /*!
    * \brief Initialize the distance vector from a json file
    *
    * \param nodes
    * \param links
    *
    * \return void
    */
    void initialize_from_json(nlohmann::json nodes, nlohmann::json links);

    /*!
     * \brief "Boot" the router
     * starts sending routing table updates
     *
     * \return void
     */
    void run();

    /*!
    * \brief Overloaded << operator
    * Prints the routers routing table
    */
    friend std::ostream& operator<<(std::ostream& os,
                                    const Router& r);

  private:
    /*!
     * \brief Update all neighbours.
     * Sends the current distance vector to all neighbours
     *
     */
    void send_update();
    /*!
     * \brief Wait for all neighbours to send an update
     */
    void receive_updates();

    /*!
     * \brief Create a Protobuf object from the distance vector
     *
     * \return dive::DistanceVector
     */
    dive::DistanceVector pack_distance_vector();

    std::shared_ptr<spdlog::logger> logger_;

    std::string router_id_;

    std::string ip_address_;
    unsigned short port_;

    std::map<std::string, int> distance_vector_;
    std::map<std::string, Link> links_;

    // SERVER
    asio::io_context& io_context_;
    asio::ip::tcp::endpoint endpoint_;
    asio::ip::tcp::acceptor acceptor_;

    // CLIENT
    asio::ip::tcp::resolver resolver_;
};

#endif  // DIVE_ROUTER_ROUTER_H
