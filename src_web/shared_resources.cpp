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
#include "shared_resources.h"

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
    const std::vector<std::string> db_dates = transfers->getTimeStamps();

    for(unsigned int i=0; i<db_dates.size(); i++)
    {
        Wt::log("info") << "db_dates[" << i << "]=" << db_dates[i];
    }

    if(db_dates != current_dates)
    {
        Wt::log("info") << "getting the latest page-databases.";
        readable_content_db.close();
        const std::vector<int> crawlers_id = transfers->getIndex();
        for(unsigned int i=0; i<crawlers_id.size(); i++)
        {
            std::string path = transfers->getPath(crawlers_id[i]);
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
        current_dates = db_dates;
    }
    else
    {
        Wt::log("info") << "using latest page-databases already.";
    }
}

std::string Trokam::SharedResources::getPassword()
{
    return password;
}
