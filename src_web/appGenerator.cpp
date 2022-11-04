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

// Wt
#include <Wt/WApplication.h>
#include <Wt/WBootstrapTheme.h>
#include <Wt/WEnvironment.h>
#include <Wt/WHBoxLayout.h>
// #include <Wt/WBootstrapTheme.h>
#include <Wt/WBootstrap5Theme.h>
#include <Wt/WCssTheme.h>
#include <Wt/WLink.h>

// Json
#include <nlohmann/json.hpp>

// Trokam
#include "appGenerator.h"
#include "common.h"
#include "preferences.h"
#include "searchPage.h"
#include "topWindow.h"

// Trokam::AppGenerator::AppGenerator(Trokam::Options &opt)
Trokam::AppGenerator::AppGenerator(nlohmann::json &opt)
    :commonResources(new Trokam::SharedResources(opt))
{}

std::unique_ptr<Wt::WApplication>
    Trokam::AppGenerator::createApplication(
        const Wt::WEnvironment& env)
{
    Wt::log("info") << "WApplication -- constructor";

    Wt::log("info") << "internal path:" << env.internalPath();
    Wt::log("info") << "deployement path:" << env.deploymentPath();
    char* app_path_ptr  = getenv("APP_PATH");
    std::string app_path = app_path_ptr;
    Wt::log("info") << "app path:" << app_path;

    Wt::log("info") << "-------- paramaters --------";
    std::map<std::string, std::vector<std::string>> parameter = env.getParameterMap();
    for (auto it = parameter.begin(); it != parameter.end(); ++it)
    {
        Wt::log("info") << "parameter: " << it->first << ", " << it->second[0];
    }
    Wt::log("info") << "-------- paramaters --------";

    // const std::string term = env.getCgiValue("term");
    const std::vector<std::string> term = env.getParameterValues("APP_PATH");
    for(auto e: term)
    {
        Wt::log("info") << "term:'" << e << "'";
    }

    std::string cookie_preferences;

    if(env.getCookie("preferences"))
    {
        cookie_preferences= *(env.getCookie("preferences"));
        Wt::log("info") << "createApplication -- cookie -- preferences:" << cookie_preferences;
    }
    else
    {
        Wt::log("info") << "cookie -- preferences = NULL";
    }

    // Trokam::Preferences preferences(cookie_preferences);
    Trokam::Preferences user_settings(cookie_preferences);
    Wt::log("info") << "preferences.getTheme()=" << user_settings.getTheme();

    /**
     * Instantiate the application. object.
     **/
    auto app = std::make_unique<Wt::WApplication>(env);

    Wt::log("info") << "appRoot: '" << app->appRoot() << "'";

    /**
     * Verifying approot directory.
     **/
    if (app->appRoot().empty())
    {
        Wt::log("error") << WARNING_APPROOT_EMPTY;
    }

    // Using Bootstrap CSS version 5.
    auto bootstrapTheme = std::make_shared<Wt::WBootstrap5Theme>();
    app->setTheme(bootstrapTheme);

    /**
     * Additional stylesheet.
     **/
    app->useStyleSheet("/style/trokam_common.css");

    if(user_settings.getTheme() == Trokam::Theme::LIGHT)
    {
        app->useStyleSheet("/style/trokam_light.css");
    }
    else
    {
        app->useStyleSheet("/style/trokam_dark.css");
    }

    /**
     * Load text bundles.
     **/
    // app->messageResourceBundle().use(app->appRoot() + "report");
    app->messageResourceBundle().use(app->appRoot() + "text");
    // app->messageResourceBundle().use(app->appRoot() + "src");

    /**
     * Add the only one widget in the application layout.
     **/
    /*
    auto layout = app->root()->setLayout(std::make_unique<Wt::WHBoxLayout>());
    layout->setPreferredImplementation(Wt::LayoutImplementation::JavaScript);
    layout->setContentsMargins(0, 0, 0, 0);
    // layout->addWidget(std::make_unique<Trokam::TopWindow>(commonResources, app.get()));
    layout->addWidget(std::make_unique<Trokam::SearchPage>(commonResources, app.get()));
    */

    app->root()->addWidget(
        std::make_unique<Trokam::SearchPage>(
            commonResources, app.get()));

    /**
     * Set web site title.
     **/
    app->setTitle("Trokam");

    return app;
}
