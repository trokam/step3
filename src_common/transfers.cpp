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
 * transfers=> \d crawlers
 *               Table "public.crawlers"
 *  Column |  Type   | Collation | Nullable | Default
 * --------+---------+-----------+----------+---------
 *  id     | integer |           | not null |
 *  extra  | text    |           |          |
 * Indexes:
 *     "crawlers_pkey" PRIMARY KEY, btree (id)
 *
 * transfers=> \d directories
 *                          Table "public.directories"
 *    Column    |            Type             | Collation | Nullable | Default
 * -------------+-----------------------------+-----------+----------+---------
 *  id          | integer                     |           | not null |
 *  crawlers_id | integer                     |           |          |
 *  date        | timestamp without time zone |           |          |
 *  path        | text                        |           |          |
 *  extra       | text                        |           |          |
 * Indexes:
 *     "directories_pkey" PRIMARY KEY, btree (id)
 * Foreign-key constraints:
 *     "fk_crawlers" FOREIGN KEY (crawlers_id) REFERENCES crawlers(id)
 **/

Trokam::Transfers::Transfers(nlohmann::json &config)
{
    const std::string host = config["transfers"]["host"];
    const std::string port;
    const std::string name = config["transfers"]["name"];
    const std::string user = config["transfers"]["user"];

    m_db.reset(new Trokam::Postgresql(host, port, name, user));
}

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

int Trokam::Transfers::getMaxIndex(const int &crawlers_id)
{
    /*
    std::string sql_select;
    sql_select=  "SELECT MAX(id) ";
    sql_select+= "FROM package ";
    pqxx::result answer;
    m_db->execAnswer(sql_select, answer);
    */

    std::string sql_select;
    sql_select=  "SELECT MAX(id) ";
    sql_select+= "FROM directories ";
    sql_select+= "WHERE crawlers_id=" + std::to_string(crawlers_id) + " ";
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

std::string Trokam::Transfers::getPath(
    const int &index,
    const int &crawlers_id)
{
    /*
    std::string sql_select;
    sql_select=  "SELECT MAX(id) ";
    sql_select+= "FROM directories ";
    sql_select+= "WHERE crawlers_id='" + std::to_string(crawlers_id) + "'";
    */

    std::string sql_select;
    sql_select=  "SELECT path ";
    sql_select+= "FROM directories ";
    sql_select+= "WHERE id=" + std::to_string(index) + " ";
    sql_select+= "AND crawlers_id=" + std::to_string(crawlers_id) + " ";

    std::cout << "sql_select=" << sql_select << "\n";

    pqxx::result answer;
    m_db->execAnswer(sql_select, answer);

    std::string result;
    pqxx::result::iterator row= answer.begin();
    if(row != answer.end())
    {
        result= row[0].as(std::string());
    }
    else
    {
        // No answer.
        std::cout << "Fail, no answer to get path\n";
    }

    return result;
}

void Trokam::Transfers::insert(
    const int &node_id,
    const std::string &path,
    const std::string &volume_id)
{
    /*
    std::string sql_insert;
    sql_insert=  "INSERT INTO package(id, date, node_volume_id, in_use) ";
    sql_insert+= "VALUES(" + std::to_string(index) + ", NOW(), '" + volume_id + "', false)";
    m_db->execNoAnswer(sql_insert);
    */

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

    /*
    std::string sql_select;
    sql_select=  "SELECT id, path, extra ";
    sql_select+= "FROM directories ";
    sql_select+= "WHERE id<" + std::to_string(max_id) + " ";
    sql_select+= "AND date < (NOW() - INTERVAL '2m') ";
    */

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
