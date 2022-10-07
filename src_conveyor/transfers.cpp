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

Trokam::Transfers::Transfers(const nlohmann::json &config)
{
    const std::string host = config["transfers"]["host"];
    const std::string port;
    const std::string name = config["transfers"]["name"];

    m_db.reset(new Trokam::Postgresql(host, port, name));
}

int Trokam::Transfers::getMaxIndex()
{
    std::string sql_select;
    sql_select=  "SELECT MAX(id) ";
    sql_select+= "FROM package ";
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

void Trokam::Transfers::insert(
    const int &index,
    const std::string &volume_id)
{
    std::string sql_insert;
    sql_insert=  "INSERT INTO package(id, date, node_volume_id, in_use) ";
    sql_insert+= "VALUES(" + std::to_string(index) + ", NOW(), '" + volume_id + "', false)";
    m_db->execNoAnswer(sql_insert);
}

pqxx::result Trokam::Transfers::getPrevious()
{
    const int max_id = getMaxIndex();

    std::string sql_select;
    sql_select=  "SELECT id, node_volume_id ";
    sql_select+= "FROM package ";
    sql_select+= "WHERE id<" + std::to_string(max_id) + " ";
    sql_select+= "AND date < (NOW() - INTERVAL '2m') ";

    pqxx::result answer;
    m_db->execAnswer(sql_select, answer);
    return answer;
}

void Trokam::Transfers::remove(
    const int &index)
{
    std::string sql_delete;
    sql_delete=  "DELETE FROM package ";
    sql_delete+= "WHERE id=" + std::to_string(index);
    m_db->execNoAnswer(sql_delete);
}

