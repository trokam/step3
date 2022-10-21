/***********************************************************************
 *                            T R O K A M
 *                      trokam.com / trokam.org
 *
 * This file is part of Trokam.
 *
 * Trokam is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Trokam is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Trokam. If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

// C
#include <stdlib.h>
#include <pwd.h>
#include <stdio.h>

// C++
#include <chrono>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>

// Boost
#include <boost/filesystem.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

// Fmt
#include <fmt/chrono.h>

// Json
#include <nlohmann/json.hpp>

// Trokam
#include "cloud_control.h"
#include "file_ops.h"
#include "postgresql.h"
#include "node.h"
#include "transfers.h"

std::string current_datetime()
{
  std::time_t tt = std::time(nullptr);
  std::tm *tm = std::localtime(&tt);
  return fmt::format("{:%Y-%m-%d-%H-%M}", *tm);
}

void verify(const int &state, std::string &command)
{
    if(state != 0)
    {
        BOOST_LOG_TRIVIAL(fatal)
            << "failure during execution of command:'" << command << "'";
        // exit(1);
    }
}

int main(int argc, char *argv[])
{
    // Get the configuration file
    const std::string config_path = "/usr/local/etc/trokam/trokam.config";
    std::string text = Trokam::FileOps::read(config_path);
    nlohmann::json config = nlohmann::json::parse(text);

    const std::string AUTH_TOKEN =     config["auth_token"];
    const std::string WEBSERVER_ID =   config["webserver_id"];
    const std::string WEBSERVER_ADDR = config["webserver_addr"];

    Trokam::Transfers transfers(config);
    Trokam::CloudControl cloud_control(config);

    int state = 0;
    std::string command;

    /**************************************
     * Remove old transfers
     *************************************/
    const int crawler_index = 0;
    pqxx::result previous_transfers = transfers.getPrevious(crawler_index);

    pqxx::result::iterator row= previous_transfers.begin();
    while(row != previous_transfers.end())
    {
        const int index =          row[0].as(int());
        const std::string vol_id = row[2].as(std::string());
        std::string path =   row[1].as(std::string());

        size_t pos = path.find("/content");
        if(pos != std::string::npos)
        {
            path = path.substr(0, pos);
        }

        BOOST_LOG_TRIVIAL(info) << "index: "  << index;
        BOOST_LOG_TRIVIAL(info) << "path: "   << path;
        BOOST_LOG_TRIVIAL(info) << "vol_id: " << vol_id;

        BOOST_LOG_TRIVIAL(info) << "umount previous directory in webserver";
        command = "ssh root@" + WEBSERVER_ADDR + " umount " + path;
        state = std::system(command.c_str());
        verify(state, command);

        std::this_thread::sleep_for(std::chrono::seconds(5));

        BOOST_LOG_TRIVIAL(info) << "delete previous directory in webserver";
        command = "ssh root@" + WEBSERVER_ADDR + " rm -r " + path;
        state = std::system(command.c_str());
        verify(state, command);

        BOOST_LOG_TRIVIAL(info) << "detaching previous volume";

        cloud_control.detachVolumeFromMachine(vol_id, WEBSERVER_ID);

        BOOST_LOG_TRIVIAL(info) << "deleting previous volume";

        cloud_control.destroyVolume(vol_id);

        transfers.remove(index);

        row++;
    }

    return 0;
}

