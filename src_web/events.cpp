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

// C++
#include <ctime>

// Trokam
#include "events.h"

/**
 * A PostgreSQL database keep the timestamp of each
 * occurrence. In general it could be located
 * in a different machine than the webserver.
 **/
Trokam::Events::Events(nlohmann::json &config)
{
    const std::string host = config["events"]["host"];
    const std::string port = config["events"]["port"];
    const std::string name = config["events"]["name"];
    const std::string user = config["events"]["user"];

    m_db.reset(new Trokam::Postgresql(host, port, name, user));
}

void Trokam::Events::insertOccurrence()
{
    std::string sql_insert;
    sql_insert=  "INSERT INTO occurrence(date) ";
    sql_insert+= "VALUES(NOW())";

    m_db->execNoAnswer(sql_insert);
}
