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

// C++ system files
#include <iostream>
#include <map>
#include <string>
#include <memory>
#include <chrono>
#include <mutex>

// Vendor header files
#include "asio.hpp"
#include "spdlog/spdlog.h"
#include "nlohmann/json.hpp"
#include "dive.pb.h"

// Project include files
#include "server.h"
#include "client.h"

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

    bool up{true};

    float pof{0};

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
           unsigned short port, asio::io_context& io_context,
           std::chrono::seconds interval);
    ~Router();

    /*!
    * \brief Initialize the distance vector from a json file
    *
    * \param nodes
    * \param links
    */
    void initialize_from_json(nlohmann::json nodes, nlohmann::json links);

    /*!
     * \brief "Boot" the router
     * starts sending routing table updates and listening for updates
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
     */
    void update_neighbours();
    /*!
     * \brief Wait for all neighbours to send an update
     */
    void receive_updates();

    /*!
     * \brief Create a Protobuf object from the distance vector
     * Object will also be serialized
     *
     * \return serialized distance vector
     */
    std::string pack_distance_vector();

    std::string pack_control_message(const std::string& data);

    /*!
     * \brief Send the current distance vector to all direct neighbours
     *
     * \param update current distance vector
     */
    void update_distance_vector(dive::DistanceVector update);

    void handle_control_message(dive::ControlMessage message);

    void simulate_outage();

    static const int kInfinity{-1};

    std::shared_ptr<spdlog::logger> logger_;

    std::string router_id_;

    std::string ip_address_;
    unsigned short port_;

    std::map<std::string, int> distance_vector_;
    std::map<std::string, Link> links_;

    // Networking
    asio::io_context& io_context_;
    Server server_;
    Client client_;

    std::chrono::seconds interval_;

    std::mutex mtx_;
};

#endif  // DIVE_ROUTER_ROUTER_H
