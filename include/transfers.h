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

// Postgresql
#include <pqxx/pqxx>

// Trokam
#include "postgresql.h"

namespace Trokam
{
    class Transfers
    {
        public:

            Transfers(const nlohmann::json &config);

            int getMaxIndex();
            
            void insert(
                const int &index,
                const std::string &volume_id);

            pqxx::result getPrevious();

            void remove(
                const int &index);

        private:

            std::unique_ptr<Trokam::Postgresql> m_db;
    };
}

