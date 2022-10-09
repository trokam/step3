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
#include <vector>

// Json
#include <nlohmann/json.hpp>

// Trokam
#include "options.h"
#include "readable_content_db.h"
#include "postgresql.h"
#include "transfers.h"

namespace Trokam
{
    class SharedResources
    {
        public:

            SharedResources(nlohmann::json &value);
            ~SharedResources();

            Trokam::ReadableContentDB readable_content_db;
            void getNewDB();

        private:

            // std::unique_ptr<Trokam::Postgresql> db;
            int current_transfer = -1;
            nlohmann::json &settings;
            // Trokam::Transfers &transfers;
            std::unique_ptr<Trokam::Transfers> transfers;
    };
}
