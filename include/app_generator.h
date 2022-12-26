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

#pragma once

// Wt
#include <Wt/WApplication.h>

// Json
#include <nlohmann/json.hpp>

// Boost
#include <boost/shared_ptr.hpp>

// Trokam
#include "app_generator.h"
#include "options.h"
#include "preferences.h"
#include "shared_resources.h"

namespace Trokam
{
    class AppGenerator
    {
        public:
            AppGenerator(nlohmann::json &opt);
            std::unique_ptr<Wt::WApplication>
                createApplication(const Wt::WEnvironment& env);

        private:
            boost::shared_ptr<Trokam::SharedResources> commonResources;
    };
}
