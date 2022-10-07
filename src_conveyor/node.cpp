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
#include "node.h"

Trokam::Node::Node(const nlohmann::json &config)
{
    const std::string host = config["node"]["host"];
    const std::string port;
    const std::string name = config["node"]["name"];

    m_db.reset(new Trokam::Postgresql(host, port, name));
}

int Trokam::Node::getMaxIndex()
{
    std::string sql_select;
    sql_select=  "SELECT MAX(id) ";
    sql_select+= "FROM bundle ";
    pqxx::result answer;
    m_db->execAnswer(sql_select, answer);

    int result = -1;
    pqxx::result::iterator row= answer.begin();
    if(row != answer.end())
    {
        result= row[0].as(int());
    }
    else
    {
        // No answer.
        std::cout << "Fail, no answer to get max id\n";
    }

    return result;
}

void Trokam::Node::insertVolumeId(const std::string &volume_id)
{
    std::string date = "123";

    std::string sql_insert;
    sql_insert=  "INSERT INTO bundle(date, local_volume_id) ";
    sql_insert+= "VALUES('" + date + "', '" + volume_id + "')";
    m_db->execNoAnswer(sql_insert);
}

std::string Trokam::Node::getVolumeId(const int &index)
{
    std::string sql_select;
    sql_select=  "SELECT local_volume_id ";
    sql_select+= "FROM bundle ";
    sql_select+= "WHERE id=" + std::to_string(index);
    pqxx::result answer;
    m_db->execAnswer(sql_select, answer);

    std::string result;
    pqxx::result::iterator row= answer.begin();
    if(row != answer.end())
    {
        result= row[0].as(std::string());
        // std::cout << "local_origin_volume_id: " << local_origin_volume_id << "\n";
    }
    else
    {
        // No answer.
        exit(10);
    }

    return result;
}

