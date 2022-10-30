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
    std::cout << "start conveyor" << std::endl;

    // Get the configuration file
    const std::string config_path = "/usr/local/etc/trokam/trokam.config";
    std::string text = Trokam::FileOps::read(config_path);
    nlohmann::json config = nlohmann::json::parse(text);

    // Get current user name
    uid_t uid = geteuid();
    struct passwd *pw = getpwuid(uid);
    if(!pw)
    {
        std::cout << "fail to get current username" << std::endl;
        exit(1);
    }

    const std::string node_user = pw->pw_name;
    const int THIS_NODE_INDEX =          config["this_node_index"];
    const int REINIT_DB_DAY =            config["reinit_db_day"];
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
    int today = current_day();
    int previous_day = today;

    while(!boost::filesystem::exists(STOP_CONVEYOR))
    {
        std::cout << "\n-----------------------------------------" << std::endl;

        today = current_day();

        /**************************************
        * Check if today correspond to start
        * with a clean database.
        *************************************/

        std::cout << "today is:" << today << " previous day:" << previous_day << std::endl;
        if(today != previous_day)
        {
            if(today == REINIT_DB_DAY)
            {
                std::cout << "today correspond a reinit of the database" << std::endl;

                std::string date= current_datetime();
                std::string command = "prime > /tmp/trokam_prime_" + date + ".log";
                state = system(command.c_str());
                show_state(state, command);
            }
        }

        /**************************************
        * Generate the volume name
        *************************************/

        int max_id = node.getMaxIndex();
        // std::string local_origin_name  = fmt::format("local-{:02}-{:06}", THIS_NODE_INDEX, max_id);
        // std::string local_origin_label = fmt::format("local_{:02}_{:06}", THIS_NODE_INDEX, max_id);
        // std::string local_origin_name  = Trokam::FileOps::generateDirName("local", THIS_NODE_INDEX, max_id);
        // std::string local_origin_label = Trokam::FileOps::generateDirLabel("local", THIS_NODE_INDEX, max_id);

        /**************************************
        * Get the current origin volumeID
        *************************************/

        std::string local_origin_volume_id = node.getVolumeId(max_id);

        /**************************************
        * Index pages in current location of database
        *************************************/

        // std::string local_current_name  = fmt::format("local-{:02}-{:06}", THIS_NODE_INDEX, max_id);
        // std::string local_current_label = fmt::format("local_{:02}_{:06}", THIS_NODE_INDEX, max_id);
        std::string local_current_name  = Trokam::FileOps::generateDirName("local", THIS_NODE_INDEX, max_id);
        std::string local_current_label = Trokam::FileOps::generateDirLabel("local", THIS_NODE_INDEX, max_id);

        std::cout << "local_current_name=" << local_current_name << std::endl;
        std::cout << "local_current_label=" << local_current_label << std::endl;

        for(unsigned int i=0; i<INDEXING_CYCLES; i++)
        {
            std::cout << "crawling cycle:" << i << std::endl;

            std::string date= current_datetime();
            std::string command =
                "trokam --action index --cycles 1 --db-content /mnt/" +
                local_current_label + "/content > /tmp/trokam_indexing_" + date + ".log";

            state = system(command.c_str());
            show_state(state, command);

            if(boost::filesystem::exists(STOP_CONVEYOR))
            {
                break;
            }
        }

        /**************************************
        * Generate the names of destination directories
        *************************************/

        max_id++;

        // std::string transfer_node_name  = fmt::format("node-{:02}-{:06}", THIS_NODE_INDEX, max_id);
        // std::string transfer_node_label = fmt::format("node_{:02}_{:06}", THIS_NODE_INDEX, max_id);
        std::string transfer_node_name  = Trokam::FileOps::generateDirName("node", THIS_NODE_INDEX, max_id);
        std::string transfer_node_label = Trokam::FileOps::generateDirLabel("node", THIS_NODE_INDEX, max_id);

        std::cout << "transfer_node_name:'" << transfer_node_name << "'" << std::endl;
        std::cout << "transfer_node_label:'" << transfer_node_label << "'" << std::endl;

        // std::string local_destination_name  = fmt::format("local-{:06}", max_id);
        // std::string local_destination_label = fmt::format("local_{:06}", max_id);
        std::string local_destination_name  = Trokam::FileOps::generateDirName("local", THIS_NODE_INDEX, max_id);
        std::string local_destination_label = Trokam::FileOps::generateDirLabel("local", THIS_NODE_INDEX, max_id);

        std::cout << "local_destination_name=" << local_destination_name << "'" << std::endl;
        std::cout << "local_destination_label=" << local_destination_label << "'" << std::endl;

        /**************************************
        * Get database size
        *************************************/

        const std::string content_path = "/mnt/" + local_current_label;
        const int current_content_size = getSize(content_path);

        // With room to grow
        const int local_new_size = current_content_size + 3;

        // With a margin
        const int transfer_new_size = current_content_size + 1;

        std::cout << "local_new_size:'" << local_new_size << "'" << std::endl;
        std::cout << "transfer_new_size:'" << transfer_new_size << "'" << std::endl;

        /**************************************
        * Create local volume
        *************************************/

        std::cout << "creating local volume" << std::endl;

        std::string local_destination_volume_id =
            cloud_control.createVolume(local_new_size, local_destination_name, local_destination_label);

        node.insertVolumeId(local_destination_volume_id);

        std::cout << "local_destination_volume_id:" << local_destination_volume_id << std::endl;

        /**************************************
        * Attaching destination local volume
        *************************************/

        std::cout << "Attaching local volume to node" << std::endl;

        cloud_control.attachVolumeToMachine(local_destination_volume_id, NODE_ID);

        /**************************************
        * Copy file to transfer volume
        *************************************/

        std::string command;

        std::cout << "Make directory for local destination" << std::endl;
        command = "ssh root@127.0.0.1 mkdir -p /mnt/" + local_destination_label;
        state = std::system(command.c_str());
        verify(state, command);

        std::cout << "Mounting local destination volume" << std::endl;
        command = "ssh root@127.0.0.1 mount -o discard,defaults,noatime /dev/disk/by-id/scsi-0DO_Volume_" + local_destination_name + " /mnt/" + local_destination_label;
        state = std::system(command.c_str());
        // verify(state, command);

        std::cout << "Modifying permissions of local destination directory" << std::endl;
        command = "ssh root@127.0.0.1 chown " + node_user + ":" + node_user + " -R /mnt/" + local_destination_label;
        state = std::system(command.c_str());
        verify(state, command);

        std::cout << "Copying database from local current directory to local destination directory" << std::endl;
        command = "cp -r /mnt/" + local_current_label + "/* /mnt/" + local_destination_label;
        state = std::system(command.c_str());
        verify(state, command);

        /**************************************
        * Do some test to verify that the database in the new volume is correct.
        *************************************/


        /**************************************
        * Create transfer volume
        *************************************/

        std::cout << "creating transfer volume";
        std::cout << "transfer_node_name:" << transfer_node_name << std::endl;
        std::cout << "transfer_node_label:" << transfer_node_label << std::endl;

        std::string transfer_volume_id =
            cloud_control.createVolume(transfer_new_size, transfer_node_name, transfer_node_label);

        /**************************************
        * Attach volume to node
        *************************************/

        std::cout << "attaching volume to node" << std::endl;

        cloud_control.attachVolumeToMachine(transfer_volume_id, NODE_ID);

        /**************************************
        * Copy database to transfer volume
        *************************************/

        std::cout << "Mounting volume to transfer database" << std::endl;
        command = "ssh root@127.0.0.1 mount -o discard,defaults,noatime /dev/disk/by-id/scsi-0DO_Volume_" + transfer_node_name + " /mnt/" + transfer_node_label;
        state = std::system(command.c_str());
        // verify(state, command);

        std::cout << "Modifying permissions of transfer directory" << std::endl;
        command = "ssh root@127.0.0.1 chown " + node_user + ":" + node_user + " -R /mnt/" + transfer_node_label;
        state = std::system(command.c_str());
        verify(state, command);

        std::cout << "Copying database to transfer volume" << std::endl;
        command = "cp -r /mnt/" + local_current_label + "/content /mnt/" + transfer_node_label;
        state = std::system(command.c_str());
        verify(state, command);

        std::cout << "Unmounting transfer volume" << std::endl;
        command = "ssh root@127.0.0.1 umount /mnt/" + transfer_node_label;
        state = std::system(command.c_str());
        verify(state, command);

        std::cout << "Delete transfer directory" << std::endl;
        command = "ssh root@127.0.0.1 rm -r /mnt/" + transfer_node_label;
        state = std::system(command.c_str());
        verify(state, command);

        /**************************************
        * Detach and destroy the local origin volume
        *************************************/

        // std::cout << "unmont, detaching and destroy local origin volume";

        std::cout << "Unmounting local current volume" << std::endl;
        command = "ssh root@127.0.0.1 umount /mnt/" + local_current_label;
        state = std::system(command.c_str());
        verify(state, command);

        std::cout << "Delete mount directory of local current volume" << std::endl;
        command = "ssh root@127.0.0.1 rm -r /mnt/" + local_current_label;
        state = std::system(command.c_str());
        verify(state, command);

        cloud_control.detachVolumeFromMachine(local_origin_volume_id, NODE_ID);
        cloud_control.destroyVolume(local_origin_volume_id);

        /**************************************
        * Detach volume from node
        *************************************/

        std::cout << "Detaching volume from node" << std::endl;

        cloud_control.detachVolumeFromMachine(transfer_volume_id, NODE_ID);

        /**************************************
        * Create directory in webserver
        *************************************/

        std::cout << "Create directory in webserver" << std::endl;
        command = "ssh root@" + WEBSERVER_ADDR + " mkdir -p /mnt/" + transfer_node_label;
        state = std::system(command.c_str());
        verify(state, command);

        std::cout << "Attaching volume to webserver" << std::endl;

        cloud_control.attachVolumeToMachine(transfer_volume_id, WEBSERVER_ID);

        /**************************************
        * Verify status of the action
        *************************************/

        std::cout << "mounting volume in webserver" << std::endl;
        command = "ssh root@" + WEBSERVER_ADDR + " mount -o discard,defaults,noatime /dev/disk/by-id/scsi-0DO_Volume_" + transfer_node_name + " /mnt/" + transfer_node_label;
        state = std::system(command.c_str());
        // verify(state, command);

        std::cout << "changing permissions" << std::endl;
        command = "ssh root@" + WEBSERVER_ADDR + " chmod a+wr -R /mnt/" + transfer_node_label;
        state = std::system(command.c_str());
        verify(state, command);

        // Test access to files in mounted volume

        /**************************************
        * Register in transfer database the new node.
        *************************************/

        const std::string path = "/mnt/" + transfer_node_label + "/content";
        transfers.insert(THIS_NODE_INDEX, path, transfer_volume_id);

        /**************************************
        * Save today's number to check in the next loop
        * if there is  a change.
        *************************************/
        previous_day = today;
    }

    boost::filesystem::remove(STOP_CONVEYOR);
    return 0;
}

