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
#include "file_ops.h"
#include "postgresql.h"
#include "transfers.h"

int getSize(const std::string &path)
{
    size_t acc_size= 0;
    for(boost::filesystem::recursive_directory_iterator it(path);
        it!=boost::filesystem::recursive_directory_iterator();
        ++it)
    {
        if(boost::filesystem::is_regular_file(*it) && !boost::filesystem::is_symlink(*it))
        {
            acc_size+= boost::filesystem::file_size(*it);
        }
    }

    float dir_size = (float)acc_size / 1000000000.0;
    size_t result = std::ceil(dir_size);
    return result;
}

std::string current_datetime()
{
    std::time_t tt = std::time(nullptr);
    std::tm *tm = std::localtime(&tt);
    return fmt::format("{:%Y-%m-%d-%H-%M}", *tm);
}

int current_day()
{
    std::time_t tt = std::time(nullptr);
    std::tm *tm = std::localtime(&tt);
    int result = tm->tm_mday;
    return result;
}

void verify(const int &state, std::string &command)
{
    if(state != 0)
    {
        std::cout
            << "failure during execution of command:'" << command << "'" << std::endl;
        exit(1);
    }
}

void show_state(const int &state, std::string &command)
{
    if(state != 0)
    {
        std::cout
            << "failure during execution of command:'" << command << "'" << std::endl;
    }
    else
    {
        std::cout
            << "successful execution of command:'" << command << "'" << std::endl;
    }
}

int main(int argc, char *argv[])
{
    std::cout << "start pump" << std::endl;

    // Get the configuration file
    const std::string config_path = "/usr/local/etc/trokam/trokam.config";
    std::string text = Trokam::FileOps::read(config_path);
    nlohmann::json config = nlohmann::json::parse(text);

    const int THIS_NODE_INDEX =          config["this_node_index"];
    const int REINIT_DB_DAY =            config["reinit_db_day"];
    const int DB_SIZE_LIMIT =            config["db_size_limit"];
    const std::string LOCAL_DIRECTORY  = config["local_directory"];
    const std::string SERVER_DIRECTORY = config["server_directory"];
    const std::string WEBSERVER_ADDR =   config["webserver_addr"];
    const std::string WEBSERVER_USER =   config["webserver_user"];
    const unsigned int INDEXING_CYCLES = config["indexing_cycles"];

    Trokam::Transfers transfers(config);

    int state = 0;
    const std::string STOP_PUPM = "/tmp/stop_pump";
    int today = current_day();
    int previous_day = today;

    while(!boost::filesystem::exists(STOP_PUPM))
    {
        std::cout << "\n-----------------------------------------" << std::endl;
        today = current_day();

        int db_size_gb = getSize(LOCAL_DIRECTORY);
        std::cout << "db size is:" << db_size_gb << std::endl;

        /**************************************
        * Check if today correspond to start
        * with a clean database.
        *************************************/

        std::cout << "today is:" << today << " previous day:" << previous_day << std::endl;
        if(today != previous_day)
        {
            if((today == REINIT_DB_DAY) || (db_size_gb > DB_SIZE_LIMIT))
            {
                std::cout << "reinit of the database" << std::endl;

                // std::string date= current_datetime();
                // std::string command = "prime > /tmp/trokam_prime_" + date + ".log";
                // state = system(command.c_str());
                // show_state(state, command);

                // trokam --action clean --db-content /some_directory/content/
                // trokam --action init --seeds-file /usr/local/etc/trokam/seeds.config

                std::string command;

                command = "trokam --action clean --db-content " + LOCAL_DIRECTORY;
                state = system(command.c_str());
                show_state(state, command);

                command = "trokam --action init --seeds-file /usr/local/etc/trokam/seeds.config";
                state = system(command.c_str());
                show_state(state, command);

                command = "mkdir -p " + LOCAL_DIRECTORY;
                state = system(command.c_str());
                show_state(state, command);
            }
        }

        /**************************************
        * Index pages
        *************************************/

        std::cout << "LOCAL_DIRECTORY=" << LOCAL_DIRECTORY << std::endl;

        for(unsigned int i=0; i<INDEXING_CYCLES; i++)
        {
            std::cout << "crawling cycle:" << i << std::endl;

            std::string date= current_datetime();
            std::string command =
                "trokam --action index --cycles 1 --db-content " +
                LOCAL_DIRECTORY + " > /tmp/trokam_indexing_" + date + ".log";

            state = system(command.c_str());
            show_state(state, command);

            if(boost::filesystem::exists(STOP_PUPM))
            {
                break;
            }
        }

        /**************************************
        * Tell the server don't use this database
        * and wait a moment for any active query to complete
        *************************************/
        std::cout << "disable database and wait" << std::endl;
        transfers.disable(THIS_NODE_INDEX);
        std::this_thread::sleep_for(std::chrono::seconds(30));

        /**************************************
        * Transfer the database to the server
        *************************************/

        std::string command;

        std::cout << "Transfer the database to the server" << std::endl;
        command = "scp -r " + LOCAL_DIRECTORY + " " +
                  WEBSERVER_USER + "@" + WEBSERVER_ADDR + ":" + SERVER_DIRECTORY;
        state = std::system(command.c_str());
        verify(state, command);

        /**************************************
        * Tell the server use the database.
        *************************************/
        std::cout << "enable database" << std::endl;
        transfers.enable(THIS_NODE_INDEX);

        /**************************************
        * Save today's number to check in the next iteration
        * if there is a change.
        *************************************/
        previous_day = today;
    }

    boost::filesystem::remove(STOP_PUPM);
    return 0;
}

