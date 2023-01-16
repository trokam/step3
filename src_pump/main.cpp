/***********************************************************************
 *                            T R O K A M
 *                       Internet Search Engine
 *                       trokam.com / trokam.org
 *
 * Copyright (C) Nicolas Slusarenko
 *               nicolas.slusarenko@trokam.com
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
#include <string>
#include <thread>

// Boost
#include <boost/filesystem.hpp>

// Fmt
#include <fmt/chrono.h>

// Json
#include <nlohmann/json.hpp>

// Trokam
#include "file_ops.h"
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

int current_hour()
{
    std::time_t tt = std::time(nullptr);
    std::tm *tm = std::localtime(&tt);
    int result = tm->tm_hour;
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

void check_disk_space(
    const std::string &file_system,
    const int &min_disk_space)
{
    boost::filesystem::space_info space_info = boost::filesystem::space(file_system);

    const float GIGABYTE = std::pow(2.0, 30.0);
    const int file_system_capacity = float(space_info.capacity)/GIGABYTE;
    const int file_system_available = float(space_info.available)/GIGABYTE;
    std::cout << "file system available:" << file_system_available
              << "[Gb] capacity:" << file_system_capacity << "[Gb]\n";

    if(file_system_available < min_disk_space)
    {
        std::cout << "disk space bellow threshold: " << min_disk_space << '\n';
        std::cout << "bye!" << std::endl;
        exit(0);
    }
}

/**
 * This command 'pump' got its name because it is pumping data to the
 * webserver. The outline of execution,
 * 1 - Read the configuration file: this command do not have command
 *     line arguments. All is specified in the configuration file.
 * 2 - If the file '/tmp/stop_pump' exists, then it quits as soon as
 *     possible. The user execute 'touch /tmp/stop_pump' to gracefully
 *     stop 'pump'.
 * 3 - Check disk space.
 * 4 - Verify if the database has grow beyond the allowed size or
 *     is the specific day to reinit. In both cases it cleans up the
 *     page-database and the link-database; then it initialises the
 *     the links-database with the links seeds.
 * 5 - Perform the indexing of pages. Here is spend most of the time.
 * 6 - Disable the database in the webserver that correspond to this
 *     crawling node.
 * 7 - Transfer the local copy of the database to the webserver.
 * 8 - Enable the database in the webserver that correspond to this
 *     crawling node.
 **/

int main(int argc, char *argv[])
{
    std::cout << "start pump" << std::endl;

    // Get the configuration file
    const std::string config_path = "/usr/local/etc/trokam/trokam.config";
    std::string text = Trokam::FileOps::read(config_path);
    nlohmann::json config = nlohmann::json::parse(text);

    const int THIS_NODE_INDEX =           config["this_node_index"];
    const int REINIT_DB_DAY =             config["reinit_db_day"];
    const int DB_SIZE_LIMIT =             config["db_size_limit"];
    const int MIN_DISK_SPACE =            3; // [Gb]
    const int HOUR_TO_TRANSFER_DB =       4;
    const int CYCLE_PERIOD =              8;
    const int CYCLE_OFFSET =              0;
    const std::string LOCAL_DIRECTORY  =  config["local_directory"];
    const std::string MNT_SERVER_DB =     config["mnt_server_db"];
    const std::string CHECK_FILE_SYSTEM = "/";
    const unsigned int INDEXING_CYCLES =  10;  // config["indexing_cycles"];

    std::cout << "THIS_NODE_INDEX:"   << THIS_NODE_INDEX << '\n';
    std::cout << "REINIT_DB_DAY:"     << REINIT_DB_DAY << '\n';
    std::cout << "DB_SIZE_LIMIT:"     << DB_SIZE_LIMIT << '\n';
    std::cout << "MIN_DISK_SPACE:"    << MIN_DISK_SPACE << '\n';
    std::cout << "LOCAL_DIRECTORY:"   << LOCAL_DIRECTORY << '\n';
    std::cout << "MNT_SERVER_DB:"     << MNT_SERVER_DB << '\n';
    std::cout << "CHECK_FILE_SYSTEM:" << CHECK_FILE_SYSTEM << '\n';
    std::cout << "INDEXING_CYCLES:"   << INDEXING_CYCLES << '\n';

    /**
     * 'transfers' is a mechanism that tells the webserver which
     * page-databases are enabled to perform the search. In general,
     * the webserver has several page-databases on which to perform
     * the query, but some of them may be disabled temporarily while
     * a crawler updates it.
     **/
    Trokam::Transfers transfers(config);

    int state = 0;
    const std::string STOP_PUPM = "/tmp/stop_pump";
    int today = current_day();
    int curr_hour = current_hour();
    int previous_day = today;
    int previous_hour = curr_hour;

    while(!boost::filesystem::exists(STOP_PUPM))
    {
        std::cout << "\n---------- new cycle ----------" << std::endl;

        /**
         * Check disk space available.
         **/
        check_disk_space(CHECK_FILE_SYSTEM, MIN_DISK_SPACE);

        /**
         * DESIGN REVIEW
         * The crawler uses two databases: a page-database and a link-database.
         * This mechanism is measuring only the size of the page-database.
         **/
        int db_size_gb = getSize(LOCAL_DIRECTORY);
        std::cout << "db size is:" << db_size_gb << "[Gb]" << std::endl;

        /**
         * Check if today correspond to start
         * with a clean database.
         **/
        today = current_day();
        int remainder = (today + CYCLE_OFFSET) % CYCLE_PERIOD;
        std::cout << "today is:" << today << " previous day:" << previous_day << std::endl;
        std::cout << "remainder:" << remainder << std::endl;
        // if(((today != previous_day) && (today == REINIT_DB_DAY)) || (db_size_gb > DB_SIZE_LIMIT))
        if(((today != previous_day) && (remainder == 0)) || (db_size_gb > DB_SIZE_LIMIT))
        {
            std::cout << current_datetime() << " -- reinit of the database" << std::endl;

            std::string command;

            command = "trokam --action clean --db-content " + LOCAL_DIRECTORY;
            state = system(command.c_str());
            show_state(state, command);

            command = "trokam --action init --seeds-file /usr/local/etc/trokam/seeds.config";
            state = system(command.c_str());
            show_state(state, command);
        }

        /**
         * Index pages.
         **/

        /**
         * DESIGN REVIEW
         * The command to index pages accept the argument 'cycles', hence
         * I wish to call the command as '--cycles=INDEXING_CYCLES' instead
         * of performing INDEXING_CYCLES loops with '--cycles=1'. But, the
         * command to index pages sometimes crashes and it does not complete
         * the total cycles expected. So, this external loop is a workaroud
         * until that is problem is solved.
         **/
        std::cout << "LOCAL_DIRECTORY=" << LOCAL_DIRECTORY << std::endl;
        for(unsigned int i=0; i<INDEXING_CYCLES; i++)
        {
            std::cout << "crawling cycle:" << i << std::endl;

            std::string date= current_datetime();
            std::string command =
                "trokam --action index "
                "--cycles 1 "
                "--db-content " + LOCAL_DIRECTORY + " "
                "> /tmp/trokam_indexing_" + date + ".log";

            state = system(command.c_str());
            show_state(state, command);

            if(boost::filesystem::exists(STOP_PUPM))
            {
                break;
            }
        }

        /**
         * Check if is the time to transfer the database.
         **/
        curr_hour = current_hour();
        std::cout << "current hour is:" << curr_hour
                  << " previous hour:" << previous_hour << std::endl;
        if((curr_hour != previous_hour) && (curr_hour == HOUR_TO_TRANSFER_DB))
        {
            /**
            * Tell the server don't use this database
            * and wait a moment for any active query to complete
            **/
            std::cout << current_datetime() << " -- disable database and wait" << std::endl;
            transfers.disable(THIS_NODE_INDEX);
            std::this_thread::sleep_for(std::chrono::seconds(20));

            /**
            * Transfer the database to the server
            * If the transfer command report an error, then it quits.
            * This database will remain as 'disabled' to avoid
            * the webserver to use a possibly corrupted database.
            **/

            /**
            * DESIGN REVIEW
            * The transfer of the content-database from the crawler node to
            * the webserver is done with the comand 'cp'. The origin is a
            * local directory and the destination is a directory of the
            * webserver mounted in a local directory. I wish not to mount
            * any directory and execute the command 'rsync' or 'scp', but
            * both fails. I tried the library 'libssh' but it fails with the
            * same error. Instead of the command 'cp' I tried
            * 'boost::filesystem::copy_file(..)' but it also fails.
            **/
            std::string command;
            std::cout << current_datetime() << " -- transfering the database to the server" << std::endl;

            std::string date = current_datetime();
            command = "cp -v -a -r " +
                    LOCAL_DIRECTORY + " " +
                    MNT_SERVER_DB + "  " +
                    "2>&1 1>/tmp/copy_" + date + ".log";
            std::cout << "command:" << command << std::endl;
            state = std::system(command.c_str());
            show_state(state, command);
            if(state != 0)
            {
                std::cout << "bye!" << std::endl;
                exit(1);
            }

            /**
            * Tell the server this database is enabled.
            **/
            std::cout << current_datetime() << " -- enable database" << std::endl;
            transfers.enable(THIS_NODE_INDEX);
        }

        /**
         * Keep today's number to check in the next iteration
         * if there is a change.
         **/
        previous_day = today;
        previous_hour = curr_hour;
    }

    boost::filesystem::remove(STOP_PUPM);
    std::cout << "bye!" << std::endl;
    return 0;
}
