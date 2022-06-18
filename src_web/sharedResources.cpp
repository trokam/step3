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

/// Wt
#include <Wt/WLogger.h>

/// Trokam::
#include "common.h"
#include "file_ops.h"
#include "sharedResources.h"

Trokam::SharedResources::SharedResources(Trokam::Options &value): settings(value)
{
    /**
    std::vector<std::vector<std::string>> clusterSettings;
    Trokam::FileOps::read(CLUSTER_FILE, clusterSettings);

    Wt::log("info") << "clusterSettings.size(): " << clusterSettings.size();

    for (size_t row=0; row<clusterSettings.size(); row++)
    {
        const std::string host= clusterSettings[row][0];
        const std::string port= clusterSettings[row][1];
        const std::string name= clusterSettings[row][2];
        const std::string user= clusterSettings[row][3];
        const std::string pass= clusterSettings[row][4];

        Wt::log("info") << "host: " << host << " port: " << port << " name: " << name << " user: " << user << " pass: " << pass;

        Trokam::Postgresql *db= new Trokam::Postgresql(host, port, name, user, pass);
        dbCluster.push_back(db);
    }

    Wt::log("info") << "database cluster size: " << dbCluster.size();
    **/
}

Trokam::SharedResources::~SharedResources()
{
    for(size_t i=0; i<dbCluster.size(); i++)
    {
        delete dbCluster[i];
    }
}
