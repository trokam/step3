/***********************************************************************
 *                            T R O K A M
 *                       Internet Search Engine
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

/// Boost
//#include <boost/bind.hpp>

// Json
#include <nlohmann/json.hpp>

/// Wt
#include <Wt/WApplication.h>
#include <Wt/WEnvironment.h>

/// Trokam
#include "appGenerator.h"
#include "common.h"
#include "file_ops.h"
#include "options.h"

int main(int argc, char **argv)
{
    /**
     * Program settings.
     **/
    // Trokam::Options opt;
    // opt.readSettings(CONFIG_FILE);

    const std::string config_path = "/usr/local/etc/trokam/trokam.config";
    std::string text = Trokam::FileOps::read(config_path);
    nlohmann::json config = nlohmann::json::parse(text);

    std::cout << "***** main *****\n";

    try
    {
        /**
         * 'AppGenerator' take the settings in the constructor.
         **/

        // Trokam::AppGenerator ag(opt);
        Trokam::AppGenerator ag(config);

        /**
         * This line block the execution, upon its return
         * the program exits.
         **/

        /*
        return Wt::WRun(
            argc, argv, boost::bind(
                &Trokam::AppGenerator::createApplication, &ag, _1));
        */

        return Wt::WRun(
            argc, argv, std::bind(
                &Trokam::AppGenerator::createApplication,
                &ag,
                std::placeholders::_1));

        // return Wt::WRun(argc, argv, &createApplication);
    }
    catch(const int &e)
    {
        // Trokam::Reporting::showGeneralError(e);
    }
}
