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
#include <array>
#include <chrono>
#include <cmath>
#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>
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

std::string execute(const char* cmd)
{
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if(!pipe)
    {
        throw std::runtime_error("popen() failed!");
    }
    while(fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
    {
        result += buffer.data();
    }
    return result;
}

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
        std::cout << "failure during execution of command:'" << command << "'\n";
        std::cout << "exit status:'" << state << "'" << std::endl;
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
    const std::string MNT_SERVER_DB =    config["mnt_server_db"];
    const unsigned int INDEXING_CYCLES = config["indexing_cycles"];

    std::cout << "THIS_NODE_INDEX:"  << THIS_NODE_INDEX << "\n";
    std::cout << "REINIT_DB_DAY:"    << REINIT_DB_DAY << "\n";
    std::cout << "DB_SIZE_LIMIT:"    << DB_SIZE_LIMIT << "\n";
    std::cout << "LOCAL_DIRECTORY:"  << LOCAL_DIRECTORY << "\n";
    std::cout << "SERVER_DIRECTORY:" << SERVER_DIRECTORY << "\n";
    std::cout << "WEBSERVER_ADDR:"   << WEBSERVER_ADDR << "\n";
    std::cout << "WEBSERVER_USER:"   << WEBSERVER_USER << "\n";
    std::cout << "MNT_SERVER_DB:"    << MNT_SERVER_DB << "\n";
    std::cout << "INDEXING_CYCLES:"  << INDEXING_CYCLES << "\n";

    Trokam::Transfers transfers(config);

    int state = 0;
    const std::string STOP_PUPM = "/tmp/stop_pump";
    int today = current_day();
    int previous_day = today;

    while(!boost::filesystem::exists(STOP_PUPM))
    {
        std::cout << "\n---------- new cycle ----------" << std::endl;
        today = current_day();

        int db_size_gb = getSize(LOCAL_DIRECTORY);
        std::cout << "db size is:" << db_size_gb << std::endl;

        /**
         * Check if today correspond to start
         * with a clean database.
         **/

        std::cout << "today is:" << today << " previous day:" << previous_day << std::endl;
        if(((today != previous_day) && (today == REINIT_DB_DAY)) || (db_size_gb > DB_SIZE_LIMIT))
        {
            std::cout << "reinit of the database" << std::endl;

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

        /**
         * Index pages.
         **/

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

        /**
         * Perform a search on common words for each language.
         * This creates indexes to speed up the search mechanism.
         **/

        const std::string words_path = "/usr/local/etc/trokam/words.json";
        std::string words_content = Trokam::FileOps::read(words_path);
        nlohmann::json words_json = nlohmann::json::parse(words_content);

        std::map<std::string, std::vector<std::string>> words =
            words_json["words"].get<std::map<std::string, std::vector<std::string>>>();

        for(auto iter= words.begin(); iter != words.end(); ++iter)
        {
            const std::string language = iter->first;
            const std::vector<std::string> term_collection = iter->second;
            for(const auto &term: term_collection)
            {
                std::string command =
                    "trokam --action search "
                    "--languages " + language + " "
                    "--db-content " + LOCAL_DIRECTORY + " "
                    "--terms " + term + " "
                    "> /tmp/word_prime.log";
                int state = system(command.c_str());
                show_state(state, command);
                std::cout << std::endl;
            }
        }

        /**
         * Tell the server don't use this database
         * and wait a moment for any active query to complete
         **/

        std::cout << "disable database and wait" << std::endl;
        transfers.disable(THIS_NODE_INDEX);
        std::this_thread::sleep_for(std::chrono::seconds(20));

        /**
         * Transfer the database to the server
         **/

        /**
        command = "rsync -Wravt " + LOCAL_DIRECTORY + " " +
                  WEBSERVER_USER + "@" + WEBSERVER_ADDR + ":" + SERVER_DIRECTORY + "/content/" +
                  " 2>&1 1>/tmp/rsync_" + date + ".log";
        **/

        /**
        boost::filesystem::copy_file(
            LOCAL_DIRECTORY,
            "/home/nicolas/website",
            boost::filesystem::copy_option::overwrite_if_exists);
        **/

        std::string command;
        std::cout << "Transfering the database to the server" << std::endl;

        std::string date = current_datetime();
        command = "cp -v -a -r " + LOCAL_DIRECTORY + " " + MNT_SERVER_DB + " 2>&1 1>/tmp/copy_" + date + ".log";
        std::cout << "command:" << command << std::endl;
        state = std::system(command.c_str());
        show_state(state, command);
        if(state != 0)
        {
            std::cout << "bye!" << std::endl;
            exit(1);
        }

        /**
         * Tell the server use the database.
         **/
        std::cout << "enable database" << std::endl;
        transfers.enable(THIS_NODE_INDEX);

        /**
         * Keep today's number to check in the next iteration
         * if there is a change.
         **/
        previous_day = today;
    }

    boost::filesystem::remove(STOP_PUPM);
    std::cout << "bye!" << std::endl;
    return 0;
}

