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
#include <functional>

// Json
#include <nlohmann/json.hpp>

// Wt
#include <Wt/WApplication.h>
#include <Wt/WEnvironment.h>

// Trokam
#include "appGenerator.h"
#include "common.h"
#include "file_ops.h"
#include "options.h"

int main(int argc, char **argv)
{
    /**
     * Program settings.
     **/
    const std::string config_path = "/usr/local/etc/trokam/trokam.config";
    std::string text = Trokam::FileOps::read(config_path);
    nlohmann::json config = nlohmann::json::parse(text);

    try
    {
        /**
         * 'AppGenerator' take the settings in the constructor.
         **/
        Trokam::AppGenerator ag(config);

        /**
         * This call block the execution, on its return
         * the program exits.
         **/
        return Wt::WRun(
            argc, argv, std::bind(
                &Trokam::AppGenerator::createApplication,
                &ag,
                std::placeholders::_1));
    }
    catch(const int &e)
    {
        // TODO: Report the exception.
    }
}
