/***********************************************************************
 *                            T R O K A M
 *                       Internet Search Engine
 *
 * Copyright (C) 2018, Nicolas Slusarenko
 *                     nicolas.slusarenko@Trokam::.com
 *
 * This file is part of Trokam::.
 *
 * Trokam:: is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Trokam:: is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Trokam::. If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

// Xapian
#include <xapian.h>

// Wt
#include <Wt/WLogger.h>

// Fmt
#include <fmt/format.h>

// Trokam
#include "common.h"
#include "file_ops.h"
#include "sharedResources.h"

/*
Trokam::SharedResources::SharedResources(
    Trokam::Options &value):
        settings(value)
*/
Trokam::SharedResources::SharedResources(
    nlohmann::json &value):
        settings(value)
{
    const std::string host;
    const std::string port;
    const std::string name = settings["transfers"]["name"];
    const std::string user = settings["transfers"]["user"];
    db.reset(new Trokam::Postgresql(host, port, name, user));

    getNewDB();

/*
    Wt::log("info") << "after database reset";
    std::cout << "after database reset\n";

    // Get the row with the largest id.
    std::string sql_select;
    sql_select=  "SELECT id ";
    sql_select+= "FROM package ";
    sql_select+= "ORDER BY id DESC ";
    sql_select+= "LIMIT 1 ";

    std::cout << "sql_select:" << sql_select << "\n";

    pqxx::result answer;
    db->execAnswer(sql_select, answer);

    int max_id = -1;
    pqxx::result::iterator row= answer.begin();
    if(row != answer.end())
    {
        max_id = row[0].as(int());
    }
    else
    {
        // No answer.
        // std::cerr << "Error, no answer.\n";
    }

    // Generate the name for the id.
    const int NODE_0 = 0;
    std::string transfer_node_name  = fmt::format("node-{:02}-{:06}", NODE_0, max_id);
    const std::string path = "/mnt/" + transfer_node_name;
    readable_content_db.open(path);
*/
}

Trokam::SharedResources::~SharedResources()
{
    /*
    for(size_t i=0; i<dbCluster.size(); i++)
    {
        delete dbCluster[i];
    }
    */
}

void Trokam::SharedResources::getNewDB()
{
    // Get the row with the largest id.
    /*
    std::string sql_select;
    sql_select=  "SELECT id ";
    sql_select+= "FROM package ";
    sql_select+= "ORDER BY id DESC ";
    sql_select+= "LIMIT 1 ";
    */

    std::string sql_select;
    sql_select=  "SELECT MAX(id) ";
    sql_select+= "FROM package ";

    pqxx::result answer;
    db->execAnswer(sql_select, answer);

    int max_id = -1;
    pqxx::result::iterator row= answer.begin();
    if(row != answer.end())
    {
        max_id = row[0].as(int());
    }
    else
    {
        // No answer.
        // std::cerr << "Error, no answer.\n";
    }

    // If there is a new transfer available, then use this one.
    if(max_id > current_transfer)
    {
        current_transfer = max_id;

        // Generate the name for the id.
        const int NODE_0 = 0;
        std::string transfer_node_name = fmt::format("node-{:02}-{:06}", NODE_0, current_transfer);
        const std::string path = "/mnt/" + transfer_node_name;
        readable_content_db.open(path);

        Wt::log("info") << "&&&&&&&&&&&&&&&&&&&& transfer:" << current_transfer << " in-use &&&&&&&&&&&&&&&&&&&&\n";

        // Mark in the database that this one is in use.
        // This enable that previous transfers, not in use anymore, could be deleted.
    }
    else
    {
        Wt::log("info") << "&&&&&&&&&&&&&&&&&&&& USING LATEST ONE ALREADY &&&&&&&&&&&&&&&&&&&&\n";
    }
}
