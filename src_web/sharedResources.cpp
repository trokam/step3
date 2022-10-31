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
    transfers.reset(new Trokam::Transfers(settings));
    getNewDB();
}

Trokam::SharedResources::~SharedResources()
{}

void Trokam::SharedResources::getNewDB()
{
    // const int crawlers_id = 0;
    // const int max_id = transfers->getMaxIndex(crawlers_id);

    const std::vector<int> crawlers_id = transfers->getCrawlersId();
    const std::vector<int> max_id = transfers->getMaxIndex(crawlers_id);

    for(unsigned int i=0; i<crawlers_id.size(); i++)
    {
        Wt::log("info") << "crawlers_if[" << i << "]=" << crawlers_id[i];
    }

    // If there is a new transfer available, then use this one.
    // if(max_id > current_transfer)
    if(max_id != current_transfer)
    {
        Wt::log("info") << "before close";
        readable_content_db.close();
        Wt::log("info") << "after close";

        Wt::log("info") << "Opening the latest content databases";
        for(unsigned int i=0; i<crawlers_id.size(); i++)
        {
            std::string path = transfers->getPath(max_id[i], crawlers_id[i]);
            Wt::log("info") << "db content path:" << path;
            if(!path.empty())
            {
                if(i == 0)
                {
                    readable_content_db.open(path);
                }
                else
                {
                    readable_content_db.add(path);
                }
            }
        }

        for(auto &e: max_id)
        {
           Wt::log("info") << "&& max_id:" << e;
        }
        current_transfer = max_id;
    }
    else
    {
        Wt::log("info") << "Using latest databases already.";
    }
}
