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
    /**
    * This is a mechanism that tells the webserver which
    * page-databases are enabled to perform the search. In general,
    * the webserver has several page-databases on which to perform
    * the query, but some of them may be disabled temporarily while
    * a crawler updates it.
    **/
    class Transfers
    {
        public:

            Transfers(nlohmann::json &config);

            std::vector<int> getIndex();

            std::vector<std::string> getTimeStamps();

            std::string getPath(
                const int &crawlers_id);

            void enable(
                const int &node_id);

            void disable(
                const int &node_id);

        private:

            std::unique_ptr<Trokam::Postgresql> m_db;
    };
}

