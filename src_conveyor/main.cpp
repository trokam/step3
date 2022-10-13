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

void show_state(const int &state, std::string &command)
{
    if(state != 0)
    {
        BOOST_LOG_TRIVIAL(info)
            << "failure during execution of command:'" << command << "'";
    }
    else
    {
        BOOST_LOG_TRIVIAL(info)
            << "successful execution of command:'" << command << "'";
    }
}

int main(int argc, char *argv[])
{
    // Get the configuration file
    const std::string config_path = "/usr/local/etc/trokam/trokam.config";
    std::string text = Trokam::FileOps::read(config_path);
    nlohmann::json config = nlohmann::json::parse(text);

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
    const std::string WEBSERVER_ID =     config["webserver_id"];
    const std::string WEBSERVER_ADDR =   config["webserver_addr"];
    const unsigned int INDEXING_CYCLES = config["indexing_cycles"];

    Trokam::Transfers transfers(config);
    Trokam::Node node(config);
    Trokam::CloudControl cloud_control(config);

    int state = 0;
    const std::string STOP_CONVEYOR = "/tmp/stop_conveyor";

    while(!boost::filesystem::exists(STOP_CONVEYOR))
    {
        /**************************************
        *      Generate date string
        *************************************/
        /*
        std::string date= current_datetime();
        if(date.length() != 16)
        {
            exit(10);
        }
        */

        /**************************************
        * Generate the volume name
        *************************************/

        int max_id = node.getMaxIndex();
        std::string local_origin_name  = fmt::format("local-{:06}", max_id);
        std::string local_origin_label = fmt::format("local_{:06}", max_id);

        /**************************************
        * Get the current origin volumeID
        *************************************/

        std::string local_origin_volume_id = node.getVolumeId(max_id);

        /**************************************
        * Index pages in current location of database
        *************************************/

        std::string local_current_name  = fmt::format("local-{:06}", max_id);
        std::string local_current_label = fmt::format("local_{:06}", max_id);

        BOOST_LOG_TRIVIAL(debug) << "local_current_name=" << local_current_name;
        BOOST_LOG_TRIVIAL(debug) << "local_current_label=" << local_current_label;

        for(unsigned int i=0; i<INDEXING_CYCLES; i++)
        {
            std::string date= current_datetime();
            std::string command =
                "trokam --action index --cycles 1 --db-content /mnt/" +
                local_current_label + "/content > /tmp/trokam_indexing_" + date + ".log";

            // BOOST_LOG_TRIVIAL(debug) << "command to perform crawling:" << command;
            state = system(command.c_str());
            show_state(state, command);
        }

        /**************************************
        * Generate the names of destination directories
        *************************************/

        max_id++;

        std::string transfer_node_name  = fmt::format("node-{:02}-{:06}", THIS_NODE_INDEX, max_id);
        std::string transfer_node_label = fmt::format("node_{:02}_{:06}", THIS_NODE_INDEX, max_id);

        BOOST_LOG_TRIVIAL(debug) << "transfer_node_name:'" << transfer_node_name << "'";
        BOOST_LOG_TRIVIAL(debug) << "transfer_node_label:'" << transfer_node_label << "'";

        std::string local_destination_name  = fmt::format("local-{:06}", max_id);
        std::string local_destination_label = fmt::format("local_{:06}", max_id);

        BOOST_LOG_TRIVIAL(debug) << "local_destination_name=" << local_destination_name << "'";
        BOOST_LOG_TRIVIAL(debug) << "local_destination_label=" << local_destination_label << "'";

        /**************************************
        * Get database size
        *************************************/

        const std::string content_path = "/mnt/" + local_current_label;
        const int current_content_size = getSize(content_path);

        // With room to grow
        const int local_new_size = current_content_size + 3;

        // With a margin
        const int transfer_new_size = current_content_size + 1;

        BOOST_LOG_TRIVIAL(debug) << "local_new_size:'" << local_new_size << "'";
        BOOST_LOG_TRIVIAL(debug) << "transfer_new_size:'" << transfer_new_size << "'";

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

        BOOST_LOG_TRIVIAL(info) << "Attaching local volume to node";

        cloud_control.attachVolumeToMachine(local_destination_volume_id, NODE_ID);

        /**************************************
        * Copy file to transfer volume
        *************************************/

        std::string command;

        // BOOST_LOG_TRIVIAL(info) << "Copy database from origin to destination volume";

        BOOST_LOG_TRIVIAL(info) << "Make directory";
        command = "ssh root@127.0.0.1 mkdir -p /mnt/" + local_destination_label;
        state = std::system(command.c_str());
        verify(state, command);

        BOOST_LOG_TRIVIAL(info) << "Mounting local volume";
        command = "ssh root@127.0.0.1 mount -o discard,defaults,noatime /dev/disk/by-id/scsi-0DO_Volume_" + local_destination_name + " /mnt/" + local_destination_label;
        state = std::system(command.c_str());
        // verify(state, command);

        BOOST_LOG_TRIVIAL(info) << "Modifying permissions";
        command = "ssh root@127.0.0.1 chown " + node_user + ":" + node_user + " -R /mnt/" + local_destination_label;
        state = std::system(command.c_str());
        verify(state, command);

        BOOST_LOG_TRIVIAL(info) << "Copying database from old volume to new one";
        command = "cp -r /mnt/" + local_origin_label + "/* /mnt/" + local_destination_label;
        state = std::system(command.c_str());
        verify(state, command);

        /**************************************
        * Do some test to verify that the database in the new volume is correct.
        *************************************/


        /**************************************
        * Create transfer volume
        *************************************/

        BOOST_LOG_TRIVIAL(info) << "creating transfer volume";
        BOOST_LOG_TRIVIAL(info) << "transfer_node_name:" << transfer_node_name;
        BOOST_LOG_TRIVIAL(info) << "transfer_node_label:" << transfer_node_label;

        std::string transfer_volume_id =
            cloud_control.createVolume(transfer_new_size, transfer_node_name, transfer_node_label);

        /**************************************
        * Attach volume to node
        *************************************/

        BOOST_LOG_TRIVIAL(info) << "attaching volume to node";

        cloud_control.attachVolumeToMachine(transfer_volume_id, NODE_ID);

        /**************************************
        * Copy database to transfer volume
        *************************************/

        // BOOST_LOG_TRIVIAL(info) << "Copying database";

        BOOST_LOG_TRIVIAL(info) << "Mounting volume";
        command = "ssh root@127.0.0.1 mount -o discard,defaults,noatime /dev/disk/by-id/scsi-0DO_Volume_" + transfer_node_name + " /mnt/" + transfer_node_label;
        state = std::system(command.c_str());
        // verify(state, command);

        BOOST_LOG_TRIVIAL(info) << "Modifying permissions";
        command = "ssh root@127.0.0.1 chown " + node_user + ":" + node_user + " -R /mnt/" + transfer_node_label;
        state = std::system(command.c_str());
        verify(state, command);

        BOOST_LOG_TRIVIAL(info) << "Copying database";
        command = "cp -r /mnt/" + local_origin_label + "/content /mnt/" + transfer_node_label;
        state = std::system(command.c_str());
        verify(state, command);

        BOOST_LOG_TRIVIAL(info) << "Unmounting volume";
        command = "ssh root@127.0.0.1 umount /mnt/" + transfer_node_label;
        state = std::system(command.c_str());
        verify(state, command);

        BOOST_LOG_TRIVIAL(info) << "Delete mount directory";
        command = "ssh root@127.0.0.1 rm -r /mnt/" + transfer_node_label;
        state = std::system(command.c_str());
        verify(state, command);

        /**************************************
        * Detach and destroy the local origin volume
        *************************************/

        // BOOST_LOG_TRIVIAL(info) << "unmont, detaching and destroy local origin volume";

        BOOST_LOG_TRIVIAL(info) << "Unmounting local origin volume";
        command = "ssh root@127.0.0.1 umount /mnt/" + local_origin_label;
        state = std::system(command.c_str());
        verify(state, command);

        BOOST_LOG_TRIVIAL(info) << "Delete mount directory of local origin volume";
        command = "ssh root@127.0.0.1 rm -r /mnt/" + local_origin_label;
        state = std::system(command.c_str());
        verify(state, command);

        cloud_control.detachVolumeFromMachine(local_origin_volume_id, NODE_ID);
        cloud_control.destroyVolume(local_origin_volume_id);

        /**************************************
        * Detach volume from node
        *************************************/

        BOOST_LOG_TRIVIAL(info) << "Detaching volume from node";

        cloud_control.detachVolumeFromMachine(transfer_volume_id, NODE_ID);

        /**************************************
        * Create directory in webserver
        *************************************/

        BOOST_LOG_TRIVIAL(info) << "Create directory in webserver";
        command = "ssh root@" + WEBSERVER_ADDR + " mkdir -p /mnt/" + transfer_node_label;
        state = std::system(command.c_str());
        verify(state, command);

        BOOST_LOG_TRIVIAL(info) << "Attaching volume to webserver";

        cloud_control.attachVolumeToMachine(transfer_volume_id, WEBSERVER_ID);

        /**************************************
        * Verify status of the action
        *************************************/

        // BOOST_LOG_TRIVIAL(info) << "mounting new volume to webserver and changing persmissions";

        BOOST_LOG_TRIVIAL(info) << "mounting volume in webserver";
        command = "ssh root@" + WEBSERVER_ADDR + " mount -o discard,defaults,noatime /dev/disk/by-id/scsi-0DO_Volume_" + transfer_node_name + " /mnt/" + transfer_node_label;
        state = std::system(command.c_str());
        // verify(state, command);

        BOOST_LOG_TRIVIAL(info) << "changing permissions";
        command = "ssh root@" + WEBSERVER_ADDR + " chmod a+wr -R /mnt/" + transfer_node_label;
        state = std::system(command.c_str());
        verify(state, command);

        // Test access to files in mounted volume

        /**************************************
        * Register in transfer database the new node.
        *************************************/

        const std::string path = "/mnt/" + transfer_node_label + "/content";
        transfers.insert(THIS_NODE_INDEX, path, transfer_volume_id);
    }

    boost::filesystem::remove(STOP_CONVEYOR);
    return 0;
}

