/***********************************************************************
 *                            T R O K A M
 *                       Internet Search Engine
 *
 * Copyright (C) 2022, Nicolas Slusarenko
 *                     nicolas.slusarenko@trokam.com
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
#include <string>
#include <vector>

// Postgresql
#include <pqxx/pqxx>

/**
 * Keep the connection with and perform the SQL sentences
 * in the PostgreSQL database.
 */
namespace Trokam
{
    class Postgresql
    {
        public:

            Postgresql();

            Postgresql(
                const std::string &host = "",
                const std::string &port = "",
                const std::string &name = "",
                const std::string &user = "",
                const std::string &pass = "");

            void exec_no_answer(
                const std::string &sentence);

            void exec_answer(
                const std::string &sentence,
                pqxx::result &answer);

            void exec_several(
                std::vector<std::string> &bundle);

        private:

            std::unique_ptr<pqxx::connection> m_connection;
    };
}
