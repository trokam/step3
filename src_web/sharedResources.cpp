/***********************************************************************
 *                            T R O K A M
 *                      trokam.com / trokam.org
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
    Wt::log("info") << "---- 0";
    transfers.reset(new Trokam::Transfers(settings));
    Wt::log("info") << "---- 1";
    getNewDB();
    Wt::log("info") << "---- 2";
}

Trokam::SharedResources::~SharedResources()
{}

void Trokam::SharedResources::getNewDB()
{
    /*
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
    */

    const int crawlers_id = 0;
    const int max_id = transfers->getMaxIndex(crawlers_id);

    // If there is a new transfer available, then use this one.
    if(max_id > current_transfer)
    {
        current_transfer = max_id;

        // Generate the name for the id.
        // const int NODE_0 = 0;
        // std::string transfer_node_name = fmt::format("node_{:02}_{:06}", NODE_0, current_transfer);
        // const std::string path = "/mnt/" + transfer_node_name + "/content";

        std::string path = transfers->getPath(max_id, crawlers_id);

        Wt::log("info") << "&&&&&&&&&&&&&&&&&&&& path:" << path;
        readable_content_db.open(path);

        Wt::log("info") << "&&&&&&&&&&&&&&&&&&&& transfer:" << current_transfer << " in-use &&&&&&&&&&&&&&&&&&&&\n";

        // Mark in the database that this one is in use.
        // This enable that previous transfers, not in use anymore, could be deleted.
        /**
        std::string sql_update;
        sql_update=  "UPDATE package ";
        sql_update+= "SET in_use=true ";
        sql_update+= "WHERE id=" + std::to_string(max_id);
        db->execNoAnswer(sql_update);
        */
    }
    else
    {
        Wt::log("info") << "&&&&&&&&&&&&&&&&&&&& USING LATEST ONE ALREADY &&&&&&&&&&&&&&&&&&&&\n";
    }
}
