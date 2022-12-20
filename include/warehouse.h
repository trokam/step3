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

// Postgresql
#include <pqxx/pqxx>

// Trokam
#include "postgresql.h"

namespace Trokam
{
    class Warehouse
    {
        public:

            Warehouse();

            std::tuple<std::string, int> get();

            std::vector<std::tuple<std::string, int>>
                getBundle(const int &total);

            void insertSeveralUrl(
                const std::vector<std::string> urls);

            bool isEmpty();

            void clean();

            void setIndexed(
                std::vector<std::tuple<std::string, int>> &bundle);

            std::string getUrl(const int &doc_id);

        private:

            const int MAX_LEVEL = 12;
            int m_current_level = -1;
            std::unique_ptr<Trokam::Postgresql> m_db;
    };
}
