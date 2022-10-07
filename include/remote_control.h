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

// Libcurl
#include <curl/curl.h>

// Json
#include <nlohmann/json.hpp>

namespace Trokam
{
    class RemoteControl
    {
        public:

            RemoteControl(const nlohmann::json &config);
            ~RemoteControl();
            
            std::string exec_post(
                const std::string &url,
                const std::string &data);

            std::string exec_get(
                const std::string &url);

            std::string exec_delete_vol(
                const std::string &url);

        private:
        
            CURL *curl = NULL;
            std::vector<std::string> headers;       
    };
}

