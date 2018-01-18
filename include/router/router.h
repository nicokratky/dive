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

/*
 * C++ system files
 */
#include <iostream>
#include <map>
#include <string>

/*
 * Vendor header files
 */
#include "spdlog/spdlog.h"

#include "json.hpp"

/*!
 * \class Router
 * \brief Class to simulate a router
 */
class Router {

  public:

    Router(std::string& router_id, std::string& ip_address, int port);
    Router(std::string& router_id, std::string& ip_address, int port,
           std::shared_ptr<spdlog::logger> logger);

    /*!
    * \brief Set the cost that it takes to reach a router
    *
    * \param router_id
    * \param cost
    *
    * \return void
    */
    void set_cost(std::string& router_id, int cost);

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
    * \brief Overloaded << operator
    * Prints the routers routing table
    */
    friend std::ostream& operator<<(std::ostream& os,
                                    const Router& r);

  private:
    std::shared_ptr<spdlog::logger> logger_;

    std::string router_id_;

    std::string ip_address_;
    int port_;

    std::map<std::string, int> distance_vector_;

};
