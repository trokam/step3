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
        m_db->exec_answer(sql_select, answer);

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

std::vector<std::tuple<std::string, int>> Trokam::Warehouse::get_bundle(
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
        m_db->exec_answer(sql_select, answer);

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
