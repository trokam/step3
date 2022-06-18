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

#ifndef TROKAM_SHARED_RESOURCES_H
#define TROKAM_SHARED_RESOURCES_H

// C++
#include <vector>

// Trokam
#include "options.h"
#include "grasp.h"
#include "postgresql.h"

namespace Trokam
{
    class SharedResources
    {
        public:

            SharedResources(Trokam::Options &value);
            ~SharedResources();

            Trokam::Options settings;  // ???
            // std::vector<Trokam::Postgresql*> dbCluster;

            Trokam::Grasp grasp;
    };
}

#endif /// TROKAM_SHARED_RESOURCES_H
