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

// C++
#include <ctime>

// Trokam
#include "warehouse.h"

Trokam::Warehouse::Warehouse()
{
    const std::string host;
    const std::string port;
    const std::string name = "warehouse";
    m_db.reset(new Trokam::Postgresql(host, port, name));
}

std::tuple<std::string, int> Trokam::Warehouse::get()
{
    while(m_current_level < MAX_LEVEL)
    {
        const std::string level = std::to_string(m_current_level);

        std::string sql_select;
        sql_select=  "SELECT link, doc_id ";
        sql_select+= "FROM pages ";
        sql_select+= "WHERE state=0 ";
        sql_select+= "AND level=" + level + " ";
        sql_select+= "ORDER BY random() ";
        sql_select+= "LIMIT 1";

        pqxx::result answer;
        m_db->execAnswer(sql_select, answer);

        pqxx::result::iterator row= answer.begin();
        if(row != answer.end())
        {
            const std::string link = row[0].as(std::string());
            const int doc_id       = row[1].as(int());

            std::cout << '\n';
            std::cout << "getting from level:" << m_current_level << '\n';
            std::cout << "getting URL:" << link << '\n';
            return {link, doc_id};
        }

        m_current_level++;
    }

    std::cout << "reached the max level to get an URL!" << '\n';
    return {"",-1};
}

std::vector<std::tuple<std::string, int>> Trokam::Warehouse::getBundle(
    const int &total)
{
    std::vector<std::tuple<std::string, int>> result;

    while(m_current_level < MAX_LEVEL)
    {
        const std::string level = std::to_string(m_current_level);

        std::string sql_select;
        sql_select=  "SELECT link, doc_id ";
        sql_select+= "FROM pages ";
        sql_select+= "WHERE state=0 ";
        sql_select+= "AND level=" + level + " ";
        sql_select+= "ORDER BY random() ";
        sql_select+= "LIMIT " + std::to_string(total);

        pqxx::result answer;
        m_db->execAnswer(sql_select, answer);

        pqxx::result::iterator row= answer.begin();
        while(row != answer.end())
        {
            const std::string link = row[0].as(std::string());
            const int doc_id       = row[1].as(int());

            std::cout << '\n';
            std::cout << "getting from level:" << m_current_level << '\n';
            std::cout << "getting URL:" << link << '\n';

            std::tuple<std::string, int> info = {link, doc_id};
            result.push_back(info);
            row++;
        }

        if(result.size() == 0)
        {
            std::cout << "incrementing current level:" << m_current_level << '\n';
            m_current_level++;
        }
        else
        {
            return result;
        }
    }

    std::cout << "reached the max level to get an URL!" << '\n';
    return result;
}

void Trokam::Warehouse::insertSeveralUrl(
    const std::vector<std::string> urls)
{
    const std::string level = std::to_string(m_current_level + 1);
    const int epoch = std::time(nullptr);
    std::vector<std::string> multiple_sql_insert;

    for(size_t i=0; i<urls.size(); i++)
    {
        std::string sql_insert;
        sql_insert=  "INSERT INTO pages(link, level, state, epoch) ";
        sql_insert+= "VALUES('" + urls[i] + "', ";
        sql_insert+= level + ", ";
        sql_insert+= "0, ";
        sql_insert+= std::to_string(epoch) + ") ";
        sql_insert+= "ON CONFLICT (link) ";
        sql_insert+= "DO NOTHING ";

        multiple_sql_insert.push_back(sql_insert);
    }

    m_db->execSeveral(multiple_sql_insert);
}

bool Trokam::Warehouse::isEmpty()
{
    std::string sql_select;
    sql_select=  "SELECT COUNT(*) ";
    sql_select+= "FROM pages ";

    pqxx::result answer;
    m_db->execAnswer(sql_select, answer);

    pqxx::result::iterator row= answer.begin();
    if(row != answer.end())
    {
        const int count= row[0].as(int());
        std::cout << "count: " << count << "\n";
        if(count == 0)
        {
            return true;
        }
    }
    else
    {
        // No answer.
        // std::cerr << "Error, no answer.\n";
    }

    return false;    
}

void Trokam::Warehouse::clean()
{
    std::string sql_drop;
    sql_drop = "DROP TABLE pages;";
    m_db->execNoAnswer(sql_drop);

    std::string sql_create_table;
    sql_create_table = "CREATE TABLE pages(";
    sql_create_table+= "link varchar(500) PRIMARY KEY, ";
    sql_create_table+= "doc_id serial, ";
    sql_create_table+= "level integer, ";
    sql_create_table+= "state integer, ";
    sql_create_table+= "epoch integer)";
    m_db->execNoAnswer(sql_create_table);
    
    std::string sql_create_index;
    sql_create_index = "CREATE INDEX doc_id_key ON pages(doc_id);";
    m_db->execNoAnswer(sql_create_index);

    sql_create_index = "CREATE INDEX epoch_key ON pages(epoch);";
    m_db->execNoAnswer(sql_create_index);

    // Now the database is empty, the level is -1.
    m_current_level = -1;
}

void Trokam::Warehouse::setIndexed(
    std::vector<std::tuple<std::string, int>> &bundle)
{
    std::vector<std::string> multiple_sql_update;

    for(size_t i=0; i<bundle.size(); i++)
    {
        const int doc_id = std::get<1>(bundle[i]);

        std::string sql_update;
        sql_update=  "UPDATE pages ";
        sql_update+= "SET state=1 ";
        sql_update+= "WHERE doc_id=" + std::to_string(doc_id);

        multiple_sql_update.push_back(sql_update);
    }

    m_db->execSeveral(multiple_sql_update);
}

std::string Trokam::Warehouse::getUrl(
    const int &doc_id)
{
    std::string sql_select;
    sql_select=  "SELECT link ";
    sql_select+= "FROM pages ";
    sql_select+= "WHERE doc_id=" + std::to_string(doc_id);

    pqxx::result answer;
    m_db->execAnswer(sql_select, answer);

    pqxx::result::iterator row= answer.begin();
    if(row != answer.end())
    {
        std::string result = row[0].as(std::string());
        return result;
    }
    else
    {
        // No answer.
        // std::cerr << "Error, no answer.\n";
    }

    return "";    
}