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

// Wt
#include <Wt/WApplication.h>
#include <Wt/WBootstrapTheme.h>
#include <Wt/WEnvironment.h>
#include <Wt/WHBoxLayout.h>
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

Trokam::AppGenerator::AppGenerator(nlohmann::json &opt)
    :commonResources(new Trokam::SharedResources(opt))
{}

std::unique_ptr<Wt::WApplication>
    Trokam::AppGenerator::createApplication(
        const Wt::WEnvironment& env)
{
    std::string cookie_preferences;
    if(env.getCookie("preferences"))
    {
        cookie_preferences= *(env.getCookie("preferences"));
    }
    Trokam::Preferences user_settings(cookie_preferences);

    /**
     * Instantiate the application object.
     **/
    auto app = std::make_unique<Wt::WApplication>(env);

    /**
     * Verifying approot directory.
     **/
    if (app->appRoot().empty())
    {
        Wt::log("error") << WARNING_APPROOT_EMPTY;
    }

    /**
     * Using Bootstrap CSS version 5.
     **/
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
    app->messageResourceBundle().use(app->appRoot() + "text");
    app->root()->addWidget(
        std::make_unique<Trokam::SearchPage>(
            commonResources, app.get()));

    /**
     * Set web site title.
     **/
    app->setTitle("Trokam - Internet Search Engine");

    return app;
}
