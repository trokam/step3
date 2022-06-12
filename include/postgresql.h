/***********************************************************************
 *                            T R O K A M
 *                       Internet Search Engine
 *
 * Copyright (C) 2018, Nicolas Slusarenko
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

#ifndef TROKAM_POSTGRESQL_H
#define TROKAM_POSTGRESQL_H

/// C++
#include <string>
#include <vector>

/// Boost
#include <boost/scoped_ptr.hpp>

/// Postgresql
#include <pqxx/pqxx>

/// Trokam
#include "options.h"

/**
 *
 **/
namespace Trokam
{
    class Postgresql
    {
        public:

            Postgresql(const Trokam::Options &value,
                       const int &id);

            Postgresql(const std::string &host,
                       const std::string &port,
                       const std::string &name,
                       const std::string &user,
                       const std::string &pass);

            ~Postgresql();

            void execSql(const std::string &sentence);

            void execSql(const std::string &sentence,
                               pqxx::result &answer);

            void execSql(std::vector<std::string> &bundle);

            static void extract(const pqxx::result &answer,
                                int &value);

            static void extract(const pqxx::result &answer,
                                bool &value);

        private:

            pqxx::connection *dbConnection;
    };
}

#endif /// TROKAM_POSTGRESQL_H
