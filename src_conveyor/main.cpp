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
        std::cout << "fail to get current username\n";
        exit(1);
    }

    const std::string node_user = pw->pw_name;
    const int THIS_NODE_ID =           config["this_node_id"];
    const std::string AUTH_TOKEN =     config["auth_token"];
    const std::string NODE_ID =        config["node_id"];
    const std::string WEBSERVER_ID =   config["webserver_id"];
    const std::string WEBSERVER_ADDR = config["webserver_addr"];

    Trokam::Transfers transfers(config);
    Trokam::Node node(config);
    Trokam::CloudControl cloud_control(config);

    int state = 0;

    /**************************************
     *      Generate date string
     *************************************/

    std::string date= current_datetime();
    std::cout << "date=" << date << '\n';
    if(date.length() != 16)
    {
        exit(10);
    }

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

    std::cout << "local_current_name=" << local_current_name << '\n';
    std::cout << "local_current_label=" << local_current_label << '\n';

    std::string command =
        "trokam --action index --db-content /mnt/" +
        local_current_label + "/content > /tmp/trokam_indexing_" + date + ".log";

    std::cout << "command=" << command << "\n";

    state = system(command.c_str());
    std::cout << "indexing state=" << state << '\n';

    /**************************************
     * Generate the names of destination directories
     *************************************/

    max_id++;

    std::string transfer_node_name  = fmt::format("node-{:02}-{:06}", THIS_NODE_ID, max_id);
    std::string transfer_node_label = fmt::format("node_{:02}_{:06}", THIS_NODE_ID, max_id);

    std::cout << "transfer_node_name=" << transfer_node_name << '\n';
    std::cout << "transfer_node_label=" << transfer_node_label << '\n';

    std::string local_destination_name  = fmt::format("local-{:06}", max_id);
    std::string local_destination_label = fmt::format("local_{:06}", max_id);

    std::cout << "local_destination_name=" << local_destination_name << '\n';
    std::cout << "local_destination_label=" << local_destination_label << '\n';

    /**************************************
     * Get database size
     *************************************/

    const std::string content_path = "/mnt/" + local_current_label;
    const int current_content_size = getSize(content_path);

    // With room to grow
    const int local_new_size = current_content_size + 1;

    // With a margin
    const int transfer_new_size = current_content_size + 1;

    std::cout << "local_new_size=" << local_new_size << "\n";
    std::cout << "transfer_new_size=" << transfer_new_size << "\n";

    /**************************************
     * Create local volume
     *************************************/

    std::cout << "============== Creating local volume ==============\n";

    std::string local_destination_volume_id =
        cloud_control.createVolume(local_new_size, local_destination_name, local_destination_label);

    node.insertVolumeId(local_destination_volume_id);

    /**************************************
     * Attaching destination local volume
     *************************************/

    std::cout << "============== Attaching local volume to node ==============\n";

    cloud_control.attachVolumeToMachine(local_destination_volume_id, NODE_ID);

    /**************************************
     * Copy file to transfer volume
     *************************************/

    std::cout << "============== Copy database from origin to destination volume ==============\n";

    std::cout << "Make directory\n";
    command = "ssh root@127.0.0.1 mkdir -p /mnt/" + local_destination_label;
    state = std::system(command.c_str());
    std::cout << "state=" << state << '\n';

    std::cout << "Mounting local volume\n";
    command = "ssh root@127.0.0.1 mount -o discard,defaults,noatime /dev/disk/by-id/scsi-0DO_Volume_" + local_destination_name + " /mnt/" + local_destination_label;
    state = std::system(command.c_str());
    std::cout << "state=" << state << '\n';

    std::cout << "Modifying permissions\n";
    command = "ssh root@127.0.0.1 chown " + node_user + ":" + node_user + " -R /mnt/" + local_destination_label;
    state = std::system(command.c_str());
    std::cout << "state=" << state << '\n';

    std::cout << "Copying database from old volume to new one\n";
    command = "cp -r /mnt/" + local_origin_label + "/* /mnt/" + local_destination_label;
    state = std::system(command.c_str());
    std::cout << "state=" << state << '\n';

    /**************************************
     * Do some test to verify that the database in the new volume is correct.
     *************************************/


    /**************************************
     * Create transfer volume
     *************************************/

    std::cout << "============== Creating transfer volume ==============\n";
    std::cout << "transfer_node_name= " << transfer_node_name << '\n';
    std::cout << "transfer_node_label=" << transfer_node_label << '\n';

    std::string transfer_volume_id =
        cloud_control.createVolume(transfer_new_size, transfer_node_name, transfer_node_label);

    /**************************************
     * Attach volume to node
     *************************************/

    std::cout << "============== Attaching volume to node ==============\n";

    cloud_control.attachVolumeToMachine(transfer_volume_id, NODE_ID);

    /**************************************
     * Copy database to transfer volume
     *************************************/

    std::cout << "============== Copying database ==============\n";

    std::cout << "Mounting volume\n";
    command = "ssh root@127.0.0.1 mount -o discard,defaults,noatime /dev/disk/by-id/scsi-0DO_Volume_" + transfer_node_name + " /mnt/" + transfer_node_label;
    state = std::system(command.c_str());
    std::cout << "state=" << state << '\n';

    std::cout << "Modifying permissions\n";
    command = "ssh root@127.0.0.1 chown " + node_user + ":" + node_user + " -R /mnt/" + transfer_node_label;
    state = std::system(command.c_str());
    std::cout << "state=" << state << '\n';

    std::cout << "Copying database\n";
    command = "cp -r /mnt/" + local_origin_label + "/content /mnt/" + transfer_node_label;

    state = std::system(command.c_str());
    std::cout << "state=" << state << '\n';

    std::cout << "Unmounting volume\n";
    command = "ssh root@127.0.0.1 umount /mnt/" + transfer_node_label;
    state = std::system(command.c_str());
    std::cout << "state=" << state << '\n';

    std::cout << "Delete mount directory\n";
    command = "ssh root@127.0.0.1 rm -r /mnt/" + transfer_node_label;
    state = std::system(command.c_str());
    std::cout << "state=" << state << '\n';

    /**************************************
     * Detach and destroy the local origin volume
     *************************************/

    std::cout << "============== Unmont, detaching and destroy local origin volume ==============\n";

    std::cout << "Unmounting local origin volume\n";
    command = "ssh root@127.0.0.1 umount /mnt/" + local_origin_label;
    state = std::system(command.c_str());
    std::cout << "state=" << state << '\n';

    std::cout << "Delete mount directory of local origin volume\n";
    command = "ssh root@127.0.0.1 rm -r /mnt/" + local_origin_label;
    state = std::system(command.c_str());
    std::cout << "state=" << state << '\n';

    cloud_control.detachVolumeFromMachine(local_origin_volume_id, NODE_ID);
    cloud_control.destroyVolume(local_origin_volume_id);

    /**************************************
     * Detach volume from node
     *************************************/

    std::cout << "============== Detaching volume from node ==============\n";

    cloud_control.detachVolumeFromMachine(transfer_volume_id, NODE_ID);

    /**************************************
     * Create directory in webserver
     *************************************/

    std::cout << "============== Create directory in webserver ==============\n";

    std::cout << "Creating directory in webserver\n";
    command = "ssh root@" + WEBSERVER_ADDR + " mkdir -p /mnt/" + transfer_node_label;
    state = std::system(command.c_str());
    std::cout << "state=" << state << '\n';

    std::cout << "============== Attaching volume to webserver ==============\n";

    cloud_control.attachVolumeToMachine(transfer_volume_id, WEBSERVER_ID);

    /**************************************
     * Verify status of the action
     *************************************/

    std::cout << "============== Mounting volume to webserver ==============\n";

    std::cout << "Mounting volume en webserver\n";
    command = "ssh root@" + WEBSERVER_ADDR + " mount -o discard,defaults,noatime /dev/disk/by-id/scsi-0DO_Volume_" + transfer_node_name + " /mnt/" + transfer_node_label;
    state = std::system(command.c_str());
    std::cout << "state=" << state << '\n';

    std::cout << "Changing permissions\n";
    command = "ssh root@" + WEBSERVER_ADDR + " chmod a+wr -R /mnt/" + transfer_node_label;
    state = std::system(command.c_str());
    std::cout << "state=" << state << '\n';

    // Test access to files in mounted volume

    /**************************************
     * Register in transfer database the new node.
     *************************************/

    // transfers.insert(max_id, transfer_volume_id);

    const std::string path = "/mnt/" + transfer_node_label + "/content";
    transfers.insert(THIS_NODE_ID, path, transfer_volume_id);

    /**************************************
     * Remove old transfers
     *************************************/

    /**

    pqxx::result previous_transfers = transfers.getPrevious();

    pqxx::result::iterator row_f= previous_transfers.begin();
    while(row_f != previous_transfers.end())
    {
        const int index = row_f[0].as(int());
        std::string vol_id = row_f[1].as(std::string());

        std::cout << "index web: " << index << '\n';
        std::cout << "vol_id: " << vol_id << '\n';

        std::string previous_transfer_label = fmt::format("node_{:02}_{:06}", THIS_NODE_ID, index);

        std::cout << "============== Remove previous in webserver ==============\n";

        std::cout << "Umount previous directory in webserver\n";
        command = "ssh root@" + WEBSERVER_ADDR + " umount /mnt/" + previous_transfer_label;
        state = std::system(command.c_str());
        std::cout << "state=" << state << '\n';

        std::cout << "Delete previous directory in webserver\n";
        command = "ssh root@" + WEBSERVER_ADDR + " rm -r /mnt/" + previous_transfer_label;
        state = std::system(command.c_str());
        std::cout << "state=" << state << '\n';

        std::cout << "Detaching previous volume\n";

        cloud_control.detachVolumeFromMachine(vol_id, WEBSERVER_ID);

        std::cout << "Deleting previous volume\n";

        cloud_control.destroyVolume(vol_id);

        transfers.remove(index);

        row_f++;
    }

    **/

    return 0;
}

