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

// C++
#include <chrono>
#include <ctime>
#include <cstdlib>
#include <iostream>
#include <string>
#include <sstream>
#include <thread>
#include <vector>

// Trokam
#include "cloud_control.h"

Trokam::CloudControl::CloudControl(
    const nlohmann::json &config)
    :remote_control(config)
{} 

std::string Trokam::CloudControl::createVolume(
    const int &size_gb,
    const std::string &name,
    const std::string &label)
{
    const std::string url = "https://api.digitalocean.com/v2/volumes";

    nlohmann::json payload;
    payload["size_gigabytes"]=   size_gb;
    payload["name"]=             name;
    payload["description"]=      "nothing";
    payload["region"]=           "ams3";
    payload["filesystem_type"]=  "ext4";
    payload["filesystem_label"]= label;
    
    // std::cout << "payload.dump\n" << payload.dump() << '\n';

    const std::string reply = remote_control.exec_post(url, payload.dump());
    const nlohmann::json answer = nlohmann::json::parse(reply);
    
    // std::cout << "answer\n" << answer.dump(4) << '\n';
    std::string result = answer["volume"]["id"];    
    waitToCompleteAction(result);   
    
    return result;
    // std::cout << "local_destination_volume_id:" << local_destination_volume_id << '\n';    
}

void Trokam::CloudControl::attachVolumeToMachine(
    const std::string &volume_id,
    const std::string &machine_id)
{
    const std::string url= "https://api.digitalocean.com/v2/volumes/" + volume_id + "/actions";

    nlohmann::json payload;
    payload["type"]=         "attach";
    payload["droplet_id"]=   machine_id;
    payload["region"]=       "ams3";

    remote_control.exec_post(url, payload.dump());
    waitToCompleteAction(volume_id);       
}

void Trokam::CloudControl::detachVolumeFromMachine(
    const std::string &volume_id,
    const std::string &machine_id)
{
    const std::string url= "https://api.digitalocean.com/v2/volumes/" + volume_id + "/actions";

    nlohmann::json payload;
    payload["type"]=         "detach";
    payload["droplet_id"]=   machine_id;
    payload["region"]=       "ams3";

    remote_control.exec_post(url, payload.dump());
    waitToCompleteAction(volume_id);       
}

void Trokam::CloudControl::destroyVolume(
    const std::string &volume_id)
{
    const std::string url= "https://api.digitalocean.com/v2/volumes/" + volume_id;
    const std::string reply= remote_control.exec_delete_vol(url);
    std::cout << "reply:" << reply << '\n';
}

void Trokam::CloudControl::waitToCompleteAction(
    const std::string &volume_id)
{
    bool completed = false;
    while(!completed)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));            
        const std::string url= "https://api.digitalocean.com/v2/volumes/" + volume_id + "/actions";
        auto reply= remote_control.exec_get(url);
        auto answer= nlohmann::json::parse(reply);
        if(isCompleted(answer))
        {
            completed= true;
        }
        else
        {
            std::cout << "waiting for action to complete ...\n";
        }
    }
    
    std::this_thread::sleep_for(std::chrono::seconds(5));            
}

bool Trokam::CloudControl::isCompleted(nlohmann::json &bunch)
{
    for(nlohmann::json::iterator it = bunch["actions"].begin();
        it != bunch["actions"].end();
        ++it)
    {
        // std::cout << *it << '\n';
        // std::cout << *it["status"] << '\n';
        nlohmann::json detail= *it;
        std::cout << detail["status"] << '\n';
        std::string status= detail["status"];
        if(status != "completed")
        {
            return false;
        }
    }
    return true;
}


