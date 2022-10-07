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

#pragma once

// C++
#include <memory>
#include <iostream>
#include <string>
#include <vector>

// Json
#include <nlohmann/json.hpp>

// Trokam
#include "remote_control.h"

namespace Trokam
{
    class CloudControl
    {
        public:

            CloudControl(const nlohmann::json &config);

            std::string createVolume(
                const int &size_gb,
                const std::string &name,
                const std::string &label);

            void attachVolumeToMachine(
                const std::string &volume_id,
                const std::string &machine_id);

            void detachVolumeFromMachine(
                const std::string &volume_id,
                const std::string &machine_id);

            void destroyVolume(
                const std::string &volume_id);
                
        private:

            RemoteControl remote_control;
            
            void waitToCompleteAction(const std::string &volume_id);
            bool isCompleted(nlohmann::json &bunch);
    };
}

