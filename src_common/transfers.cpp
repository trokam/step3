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
#include <ctime>

// Trokam
#include "transfers.h"

/**
 * A PostgreSQL database keep the state of each
 * page-database. In general it could be located
 * in a different machine than the webserver.
 **/
Trokam::Transfers::Transfers(nlohmann::json &config)
{
    const std::string host = config["transfers"]["host"];
    const std::string port;
    const std::string name = config["transfers"]["name"];
    const std::string user = config["transfers"]["user"];

    m_db.reset(new Trokam::Postgresql(host, port, name, user));
}

std::vector<int> Trokam::Transfers::getIndex()
{
    std::string sql_select;
    sql_select=  "SELECT crawler_id ";
    sql_select+= "FROM dbcontent ";
    sql_select+= "WHERE enabled=true ";
    pqxx::result answer;
    m_db->execAnswer(sql_select, answer);

    std::vector<int> result;
    pqxx::result::iterator row= answer.begin();
    while(row != answer.end())
    {
        const int crawler_id = row[0].as(int());
        result.push_back(crawler_id);
        row++;
    }

    return result;
}

std::vector<std::string> Trokam::Transfers::getTimeStamps()
{
    std::string sql_select;
    sql_select=  "SELECT date ";
    sql_select+= "FROM dbcontent ";
    sql_select+= "WHERE enabled=true ";
    pqxx::result answer;
    m_db->execAnswer(sql_select, answer);

    std::vector<std::string> result;
    pqxx::result::iterator row= answer.begin();
    while(row != answer.end())
    {
        const std::string date = row[0].as(std::string());
        result.push_back(date);
        row++;
    }

    return result;
}

std::string Trokam::Transfers::getPath(
    const int &crawlers_id)
{
    std::string sql_select;
    sql_select=  "SELECT path ";
    sql_select+= "FROM dbcontent ";
    sql_select+= "WHERE crawler_id=" + std::to_string(crawlers_id) + " ";
    sql_select+= "AND enabled=true ";

    pqxx::result answer;
    m_db->execAnswer(sql_select, answer);

    std::string result;
    pqxx::result::iterator row= answer.begin();
    if(row != answer.end())
    {
        result= row[0].as(std::string());
    }

    return result;
}

void Trokam::Transfers::enable(
    const int &node_id)
{
    std::string sql_update;
    sql_update=  "UPDATE dbcontent ";
    sql_update+= "SET date=NOW(), enabled=true ";
    sql_update+= "WHERE crawler_id=" + std::to_string(node_id) + " ";

    m_db->execNoAnswer(sql_update);
}

void Trokam::Transfers::disable(
    const int &node_id)
{
    std::string sql_update;
    sql_update=  "UPDATE dbcontent ";
    sql_update+= "SET date=NOW(), enabled=false ";
    sql_update+= "WHERE crawler_id=" + std::to_string(node_id) + " ";

    m_db->execNoAnswer(sql_update);
}
