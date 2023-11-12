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

// Boost
#include <boost/algorithm/string.hpp>

// Trokam
#include "qa_cache.h"

Trokam::QAcache::QAcache(nlohmann::json &config)
{
    try
    {
        const std::string host = config["qa_cache"]["host"];
        const std::string port = config["qa_cache"]["port"];
        const std::string name = config["qa_cache"]["name"];
        const std::string user = config["qa_cache"]["user"];

        m_db.reset(new Trokam::Postgresql(host, port, name, user));
    }
    catch(const std::exception& e)
    {
        std::cerr << "fail in constructor of QAcache\n";
        std::cerr << "error:" << e.what() << '\n';
    }
}

std::string Trokam::QAcache::getAnswer(const std::string &question)
{
    std::string escaped_question = question;
    boost::replace_all(escaped_question, "'", "''");

    std::string sql_select;
    sql_select=  "SELECT answer ";
    sql_select+= "FROM cache ";
    sql_select+= "WHERE question='" + escaped_question + "'";
    pqxx::result answer;
    m_db->execAnswer(sql_select, answer);

    pqxx::result::iterator row= answer.begin();
    while(row != answer.end())
    {
        const std::string result = row[0].as(std::string());
        return result;
    }
    return "";
}

void Trokam::QAcache::saveQA(
    const std::string &question,
    const std::string &answer)
{
    std::string escaped_question = question;
    boost::replace_all(escaped_question, "'", "''");

    std::string escaped_answer = answer;
    boost::replace_all(escaped_answer, "'", "''");

    std::string sql_select;
    sql_select=  "INSERT INTO cache(question, answer) ";
    sql_select+= "VALUES('" + escaped_question + "','" + escaped_answer + "')";
    m_db->execNoAnswer(sql_select);
}
