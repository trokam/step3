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
 *
 **/

Trokam::Transfers::Transfers(nlohmann::json &config)
{
    const std::string host = config["transfers"]["host"];
    const std::string port;
    const std::string name = config["transfers"]["name"];
    const std::string user = config["transfers"]["user"];

    m_db.reset(new Trokam::Postgresql(host, port, name, user));
}

/*
std::vector<int> Trokam::Transfers::getCrawlersId()
{
    std::string sql_select;
    sql_select=  "SELECT id ";
    sql_select+= "FROM crawlers ";
    pqxx::result answer;
    m_db->execAnswer(sql_select, answer);

    std::vector<int> result;
    pqxx::result::iterator row= answer.begin();
    while(row != answer.end())
    {
        const int id = row[0].as(int());
        result.push_back(id);
        row++;
    }

    return result;
}
*/

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

/*
std::vector<int> Trokam::Transfers::getMaxIndex(
    const std::vector<int> &crawlers_id)
{
    std::vector<int> result;
    for(unsigned int i=0; i<crawlers_id.size(); i++)
    {
        int index = getMaxIndex(crawlers_id[i]);
        result.push_back(index);
    }
    return result;
}
*/

std::string Trokam::Transfers::getPath(
    const int &crawlers_id)
{
    std::string sql_select;
    sql_select=  "SELECT path ";
    sql_select+= "FROM dbcontent ";
    sql_select+= "WHERE crawler_id=" + std::to_string(crawlers_id) + " ";
    sql_select+= "AND enabled=true ";

    std::cout << "sql_select=" << sql_select << "\n";

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

    std::cout << "sql_update:" << sql_update << "\n";

    m_db->execNoAnswer(sql_update);
}

void Trokam::Transfers::disable(
    const int &node_id)
{
    std::string sql_update;
    sql_update=  "UPDATE dbcontent ";
    sql_update+= "SET date=NOW(), enabled=false ";
    sql_update+= "WHERE crawler_id=" + std::to_string(node_id) + " ";

    std::cout << "sql_update:" << sql_update << "\n";

    m_db->execNoAnswer(sql_update);
}

/**
void Trokam::Transfers::insert(
    const int &node_id,
    const std::string &path,
    const std::string &volume_id)
{
    std::string sql_insert;
    sql_insert=  "INSERT INTO directories(crawlers_id, date, path, extra) ";
    sql_insert+= "VALUES(" + std::to_string(node_id) + ", NOW(), '" + path + "', '" + volume_id + "')";

    std::cout << "sql_insert:" << sql_insert << "\n";

    m_db->execNoAnswer(sql_insert);
}

pqxx::result Trokam::Transfers::getPrevious(
    const int &crawler_index)
{
    const int max_id = getMaxIndex(crawler_index);

    std::string sql_select;
    sql_select=  "SELECT id, path, extra ";
    sql_select+= "FROM directories ";
    sql_select+= "WHERE id<" + std::to_string(max_id) + " ";
    sql_select+= "AND crawlers_id=" + std::to_string(crawler_index) + " ";
    sql_select+= "AND date < (NOW() - INTERVAL '2m') ";

    std::cout << "sql_select:" << sql_select << "\n";

    pqxx::result answer;
    m_db->execAnswer(sql_select, answer);
    return answer;
}

void Trokam::Transfers::remove(
    const int &index)
{
    std::string sql_delete;
    sql_delete=  "DELETE FROM directories ";
    sql_delete+= "WHERE id=" + std::to_string(index);
    m_db->execNoAnswer(sql_delete);
}
**/