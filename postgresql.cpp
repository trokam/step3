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
#include <iostream>

// Trokam
#include "postgresql.h"

Trokam::Postgresql::Postgresql()
{}

Trokam::Postgresql::Postgresql(
    const std::string &host,
    const std::string &port,
    const std::string &name,
    const std::string &user,
    const std::string &pass)
{
    std::string conn_param;

    if(!host.empty())
    {
        conn_param+= "host=" + host + " ";
    }

    if(!port.empty())
    {
        conn_param+= "port=" + port + " ";
    }

    if(!name.empty())
    {
        conn_param+= "dbname=" + name + " ";
    }

    if(!user.empty())
    {
        conn_param+= "user=" + user + " ";
    }

    if(!pass.empty())
    {
        conn_param+= "password=" + pass + " ";
    }

    try
    {
        m_connection.reset(new pqxx::connection(conn_param));
        std::cout << "connected to database \n";
        std::cout << "backend version: " << m_connection->server_version() << "\n";
        std::cout << "protocol version: " << m_connection->protocol_version() << std::endl;
    }
    catch(const std::exception &e)
    {
        std::cerr << "FAIL: " << e.what() << std::endl;
    }
}

void Trokam::Postgresql::execNoAnswer(
    const std::string &sentence)
{
    try
    {
        // Begin a transaction acting on our current connection.
        pqxx::work T(*m_connection, "execute_no_answer");

        // Perform a query on the database, there are no results.
        T.exec(sentence);

        // This is not really necessary if no modifications are made
        // to commit.
        T.commit();
    }
    catch(const pqxx::sql_error &e)
    {
        std::cerr << "FAIL: " << e.what() << std::endl;
    }
    catch(const std::exception &e)
    {
        std::cerr << "FAIL: " << e.what() << std::endl;
    }
    catch(...)
    {
        std::cerr << "FAIL: unhandled exception\n";
    }
}

void Trokam::Postgresql::execAnswer(
    const std::string &sentence,
    pqxx::result &answer)
{
    try
    {
        // Begin a transaction acting on our current connection.
        pqxx::work T(*m_connection, "exec_answer");

        // Perform a query on the database, storing result in answer.
        answer= T.exec(sentence);

        // This is not really necessary if no modifications are made
        // to commit.
        T.commit();
    }
    catch(const pqxx::sql_error &e)
    {
        std::cerr << "FAIL: " << e.what() << std::endl;
    }
    catch(const std::exception &e)
    {
        std::cerr << "FAIL: " << e.what() << std::endl;
    }
    catch(...)
    {
        std::cerr << "FAIL: unhandled exception\n";
    }
}

void Trokam::Postgresql::execSeveral(
    std::vector<std::string> &bundle)
{
    try
    {
        // Begin a transaction acting on our current connection.
        pqxx::work T(*m_connection, "execute_several_no_answer");

        for(std::vector<std::string>::iterator it= bundle.begin(); it!=bundle.end(); ++it)
        {
            const std::string sentence= *it;
            T.exec(sentence);
        }

        T.commit();
    }
    catch(const pqxx::sql_error &e)
    {
        std::cerr << "FAIL: " << e.what() << std::endl;
    }
    catch(const std::exception &e)
    {
        std::cerr << "FAIL: " << e.what() << std::endl;
    }
    catch(...)
    {
        std::cerr << "FAIL: unhandled exception\n";
    }
}
