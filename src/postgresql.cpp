/***********************************************************************
 *                            T R O K A M
 *                       Internet Search Engine
 *
 * Copyright (C) 2018, Nicolas Slusarenko
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

/// C++
#include <cassert>
#include <iostream>

/// Trokam
#include "common.h"
#include "exception.h"
#include "postgresql.h"

Trokam::Postgresql::Postgresql(const Trokam::Options &value,
                               const int &id)
{
    std::string connParameters;

    if(id == DB_TEXT_SEARCH)
    {
        if(value.dbHost() != "")
        {
            connParameters+= "host=" + value.dbHost() + " ";
        }

        if(value.dbName() != "")
        {
            connParameters+= "dbname=" + value.dbName() + " ";
        }

        if(value.dbUser() != "")
        {
            connParameters+= "user=" + value.dbUser() + " ";
        }

        if(value.dbPass() != "")
        {
            connParameters+= "password=" + value.dbPass() + " ";
        }
    }

    if(id == DB_CONTROL)
    {
        if(value.dbHost() != "")
        {
            connParameters+= "host=" + value.controlHost() + " ";
        }

        if(value.dbName() != "")
        {
            connParameters+= "dbname=" + value.controlName() + " ";
        }

        if(value.dbUser() != "")
        {
            connParameters+= "user=" + value.controlUser() + " ";
        }

        if(value.dbPass() != "")
        {
            connParameters+= "password=" + value.controlPass() + " ";
        }
    }

    if(id == DB_DEPOT)
    {
        if(value.dbHost() != "")
        {
            connParameters+= "host=" + value.depotHost() + " ";
        }

        if(value.dbName() != "")
        {
            connParameters+= "dbname=" + value.depotName() + " ";
        }

        if(value.dbUser() != "")
        {
            connParameters+= "user=" + value.depotUser() + " ";
        }

        if(value.dbPass() != "")
        {
            connParameters+= "password=" + value.depotPass() + " ";
        }
    }

    try
    {
        dbConnection= new pqxx::connection(connParameters);
        std::cout << "connected to database \n";
        std::cout << "backend version: " << dbConnection->server_version() << "\n";
        std::cout << "protocol version: " << dbConnection->protocol_version() << std::endl;

    }
    catch(const std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
        throw Trokam::Exception(COULD_NOT_CONNECT_TO_DATABASE);
    }
}

Trokam::Postgresql::Postgresql(const std::string &host,
                               const std::string &port,
                               const std::string &name,
                               const std::string &user,
                               const std::string &pass)
{
    std::string connParameters;
    connParameters+= "host=" + host + " ";
    connParameters+= "port=" + port + " ";
    connParameters+= "dbname=" + name + " ";
    connParameters+= "user=" + user + " ";
    connParameters+= "password=" + pass + " ";

    try
    {
        dbConnection= new pqxx::connection(connParameters);
        std::cout << "connected to database \n";
        std::cout << "backend version: " << dbConnection->server_version() << "\n";
        std::cout << "protocol version: " << dbConnection->protocol_version() << std::endl;

    }
    catch(const std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
        throw Trokam::Exception(COULD_NOT_CONNECT_TO_DATABASE);
    }
}

Trokam::Postgresql::~Postgresql()
{
    delete dbConnection;
}

void Trokam::Postgresql::execSql(const std::string &sentence)
{
    try
    {
        /**
         * Begin a transaction acting on our current connection.
         **/
        pqxx::work T(*dbConnection, "execute_no_answer");

        /**
         * Perform a query on the database, there are no results.
         **/
        T.exec(sentence);

        /**
         * Tell the transaction that it has been successful.
         * This is not really necessary if no modifications are made
         * to commit.
         **/
        T.commit();
    }
    catch(const pqxx::sql_error &e)
    {
        throw(e);
    }
    catch(const std::exception &e)
    {
        /**
         * All exceptions thrown by libpqxx are
         * derived from std::exception.
         **/
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    catch(...)
    {
        /**
         * This is unexpected.
         **/
        std::cerr << "Unhandled exception\n";
    }
}

void Trokam::Postgresql::execSql(const std::string &sentence,
                                       pqxx::result &answer)
{
    try
    {
        /**
         * Begin a transaction acting on our current connection.
         **/
        pqxx::work T(*dbConnection, "execute");

        /**
         * Perform a query on the database, storing result in answer.
         **/
        answer= T.exec(sentence);

        /**
         * Tell the transaction that it has been successful.
         * This is not really necessary if no modifications are made
         * to commit.
         **/
        T.commit();
    }
    catch(const pqxx::sql_error &e)
    {
        throw(e);
    }
    catch(const std::exception &e)
    {
        /**
         * All exceptions thrown by libpqxx are
         * derived from std::exception.
         **/
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    catch(...)
    {
        /**
         * This is unexpected.
         **/
        std::cerr << "Unhandled exception\n";
    }
}

void Trokam::Postgresql::execSql(std::vector<std::string> &bundle)
{
    try
    {
        /**
         * Begin a transaction acting on our current connection.
         **/
        pqxx::work T(*dbConnection, "execute_several_no_answer");

        for(std::vector<std::string>::iterator it= bundle.begin(); it!=bundle.end(); ++it)
        {
            const std::string sentence= *it;
            T.exec(sentence);
        }

        T.commit();
    }
    catch(const pqxx::sql_error &e)
    {
        throw(e);
    }
    catch(const std::exception &e)
    {
        /**
         * All exceptions thrown by libpqxx are
         * derived from std::exception.
         **/
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    catch(...)
    {
        /**
         * This is unexpected.
         **/
        std::cerr << "Unhandled exception\n";
    }
}

void Trokam::Postgresql::extract(const pqxx::result &answer, int &value)
{
    const pqxx::result::const_iterator row = answer.begin();
    if (row != answer.end())
    {
        value= row[0].as(int());
    }
}

void Trokam::Postgresql::extract(const pqxx::result &answer, bool &value)
{
    const pqxx::result::const_iterator row = answer.begin();
    if (row != answer.end())
    {
        value= row[0].as(bool());
    }
}
