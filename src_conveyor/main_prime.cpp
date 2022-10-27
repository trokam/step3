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
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <iostream>
#include <string>

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

void verify(const int &state, std::string &command)
{
    if(state != 0)
    {
        BOOST_LOG_TRIVIAL(fatal)
            << "failure during execution of command:'" << command << "'";
        exit(1);
    }
}

int main(int argc, char *argv[])
{
    if(argc != 2)
    {
        std::cout << "specify either --new-db or --restart-db\n";
        exit(1);
    }

    const std::string NEW_DB = "--new-db";
    const std::string RESTART_DB = "--restart-db";

    std::string argument = argv[1];
    if(!(argument==NEW_DB) || (argument==RESTART_DB))
    {
        std::cout << "specify either --new-db or --restart-db\n";
        exit(1);
    }

    // Get the configuration file
    const std::string config_path = "/usr/local/etc/trokam/trokam.config";
    std::string text = Trokam::FileOps::read(config_path);
    nlohmann::json config = nlohmann::json::parse(text);

    if(argument == NEW_DB)
    {
        // Get current user name
        uid_t uid = geteuid();
        struct passwd *pw = getpwuid(uid);
        if(!pw)
        {
            BOOST_LOG_TRIVIAL(fatal) << "fail to get current username";
            exit(1);
        }

        const std::string node_user = pw->pw_name;
        const int THIS_NODE_INDEX =          config["this_node_index"];
        const std::string AUTH_TOKEN =       config["auth_token"];
        const std::string NODE_ID =          config["node_id"];

        Trokam::Node node(config);
        Trokam::CloudControl cloud_control(config);

        std::string command;
        int state = 0;

        /**************************************
        * Generate the volume name
        *************************************/

        int max_id = node.getMaxIndex();
        if(max_id != 0)
        {
            std::cout << "fail: database is not emptly.";
            exit(1);
        }

        max_id = 0;

        /**************************************
        * Index pages in current location of database
        *************************************/

        // std::string local_destination_name  = fmt::format("local-{:06}", max_id);
        // std::string local_destination_label = fmt::format("local_{:06}", max_id);

        std::string local_destination_name  = fmt::format("local-{:02}-{:06}", THIS_NODE_INDEX, max_id);
        std::string local_destination_label = fmt::format("local_{:02}_{:06}", THIS_NODE_INDEX, max_id);

        BOOST_LOG_TRIVIAL(debug) << "local_destination_name=" << local_destination_name << "'";
        BOOST_LOG_TRIVIAL(debug) << "local_destination_label=" << local_destination_label << "'";

        /**************************************
        * Get database size
        *************************************/

        const int local_new_size = 2;

        /**************************************
        * Create local volume
        *************************************/

        BOOST_LOG_TRIVIAL(info) << "creating local volume";

        std::string local_destination_volume_id =
            cloud_control.createVolume(local_new_size, local_destination_name, local_destination_label);

        node.insertVolumeId(local_destination_volume_id);

        BOOST_LOG_TRIVIAL(info) << "local_destination_volume_id:" << local_destination_volume_id;

        /**************************************
        * Attaching destination local volume
        *************************************/

        BOOST_LOG_TRIVIAL(info) << "attaching local volume to node";

        cloud_control.attachVolumeToMachine(local_destination_volume_id, NODE_ID);

        /**************************************
        * Copy file to transfer volume
        *************************************/

        BOOST_LOG_TRIVIAL(info) << "make directory";
        command = "ssh root@127.0.0.1 mkdir -p /mnt/" + local_destination_label;
        state = std::system(command.c_str());
        verify(state, command);

        BOOST_LOG_TRIVIAL(info) << "mounting local volume";
        command = "ssh root@127.0.0.1 mount -o discard,defaults,noatime /dev/disk/by-id/scsi-0DO_Volume_" + local_destination_name + " /mnt/" + local_destination_label;
        state = std::system(command.c_str());
        // verify(state, command);

        BOOST_LOG_TRIVIAL(info) << "modifying permissions";
        command = "ssh root@127.0.0.1 chown " + node_user + ":" + node_user + " -R /mnt/" + local_destination_label;
        state = std::system(command.c_str());
        verify(state, command);

        /**************************************
        * Initialize the content database
        *************************************/

        const std::string content_path = "/mnt/" + local_destination_label + "/content";

        command =
            "trokam --action clean --db-content " + content_path;
        BOOST_LOG_TRIVIAL(debug) << "command to perform cleaning of local database:" << command;
        state = system(command.c_str());
        verify(state, command);

        std::string date= current_datetime();

        command =
            "trokam --action init --seeds-file /usr/local/etc/trokam/seeds.config "
            "--db-content " + content_path + " > /tmp/trokam_indexing_" + date + ".log";
        BOOST_LOG_TRIVIAL(debug) << "command to perform initialization:" << command;
        state = system(command.c_str());
        verify(state, command);

    }

    /**
     *
     * Restart DB
     *
     **/

    if(argument == RESTART_DB)
    {
        // Get current user name
        uid_t uid = geteuid();
        struct passwd *pw = getpwuid(uid);
        if(!pw)
        {
            BOOST_LOG_TRIVIAL(fatal) << "fail to get current username";
            exit(1);
        }

        const std::string node_user = pw->pw_name;
        const std::string AUTH_TOKEN =       config["auth_token"];
        const std::string NODE_ID =          config["node_id"];

        Trokam::Node node(config);
        Trokam::CloudControl cloud_control(config);

        std::string command;
        int state = 0;

        /**************************************
        *      Generate date string
        *************************************/

        std::string date= current_datetime();
        if(date.length() != 16)
        {
            exit(10);
        }

        /**************************************
        * Generate the volume name
        *************************************/

        int max_id = node.getMaxIndex();
        max_id++;

        /**************************************
        * Index pages in current location of database
        *************************************/

        std::string local_destination_name  = fmt::format("local-{:06}", max_id);
        std::string local_destination_label = fmt::format("local_{:06}", max_id);

        BOOST_LOG_TRIVIAL(debug) << "local_destination_name=" << local_destination_name << "'";
        BOOST_LOG_TRIVIAL(debug) << "local_destination_label=" << local_destination_label << "'";

        /**************************************
        * Get database size
        *************************************/

        const int local_new_size = 2;

        /**************************************
        * Create local volume
        *************************************/

        BOOST_LOG_TRIVIAL(info) << "creating local volume";

        std::string local_destination_volume_id =
            cloud_control.createVolume(local_new_size, local_destination_name, local_destination_label);

        node.insertVolumeId(local_destination_volume_id);

        BOOST_LOG_TRIVIAL(info) << "local_destination_volume_id:" << local_destination_volume_id;

        /**************************************
        * Attaching destination local volume
        *************************************/

        BOOST_LOG_TRIVIAL(info) << "attaching local volume to node";

        cloud_control.attachVolumeToMachine(local_destination_volume_id, NODE_ID);

        /**************************************
        * Copy file to transfer volume
        *************************************/

        BOOST_LOG_TRIVIAL(info) << "make directory";
        command = "ssh root@127.0.0.1 mkdir -p /mnt/" + local_destination_label;
        state = std::system(command.c_str());
        verify(state, command);

        BOOST_LOG_TRIVIAL(info) << "mounting local volume";
        command = "ssh root@127.0.0.1 mount -o discard,defaults,noatime /dev/disk/by-id/scsi-0DO_Volume_" + local_destination_name + " /mnt/" + local_destination_label;
        state = std::system(command.c_str());
        // verify(state, command);

        BOOST_LOG_TRIVIAL(info) << "modifying permissions";
        command = "ssh root@127.0.0.1 chown " + node_user + ":" + node_user + " -R /mnt/" + local_destination_label;
        state = std::system(command.c_str());
        verify(state, command);

        /**************************************
        * Initialize the content database
        *************************************/

        const std::string content_path = "/mnt/" + local_destination_label + "/content";

        command =
            "trokam --action clean --db-content " + content_path;
        BOOST_LOG_TRIVIAL(debug) << "command to perform cleaning of local database:" << command;
        state = system(command.c_str());
        verify(state, command);

        command =
            "trokam --action init --seeds-file /usr/local/etc/trokam/seeds.config "
            "--db-content " + content_path + " > /tmp/trokam_indexing_" + date + ".log";
        BOOST_LOG_TRIVIAL(debug) << "command to perform initialization:" << command;
        state = system(command.c_str());
        verify(state, command);
    }

    return 0;
}

