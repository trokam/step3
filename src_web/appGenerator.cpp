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
#include "common.h"
#include "appGenerator.h"
#include "topWindow.h"
#include "searchPage.h"

// Trokam::AppGenerator::AppGenerator(Trokam::Options &opt)
Trokam::AppGenerator::AppGenerator(nlohmann::json &opt)
    :commonResources(new Trokam::SharedResources(opt))
{}

std::unique_ptr<Wt::WApplication>
    Trokam::AppGenerator::createApplication(
        const Wt::WEnvironment& env)
{
    Wt::log("info") << "WApplication -- constructor";
    std::string page_style = "light";

    // const Wt::WEnvironment& envx = app->environment();
    // std::string page_style= *(envx.getCookie("page_style"));

    // Wt::log("info") << "cookie -- page_style" << *(env.getCookie("languages"));
    // Wt::log("info") << "cookie -- page_style" << page_style;
    if(env.getCookie("page_style"))
    {
        page_style= *(env.getCookie("page_style"));
        Wt::log("info") << "cookie -- page_style:" << page_style;
    }
    else
    {
        Wt::log("info") << "cookie -- page_style = NULL";
    }


/**
std::unique_ptr<Wt::WApplication>
    createApplication(const Wt::WEnvironment& env)
{
**/

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

    /**
     * Using Bootstrap CSS version 3.
     **/
    /*
    auto bootstrapTheme = std::make_shared<Wt::WBootstrapTheme>();
    bootstrapTheme->setVersion(Wt::BootstrapVersion::v3);
    bootstrapTheme->setResponsive(true);
    app->setTheme(bootstrapTheme);
    app->useStyleSheet("resources/themes/bootstrap/3/bootstrap-theme.min.css");
    */

    /**
     * Customized Bootstrap 3 CCS.
     **/
    // app->useStyleSheet("/style/custom-bootstrap.css");
    // app->useStyleSheet("/style/custom-bootstrap.min.css");
    // app->useStyleSheet("/style/custom-bootstrap-theme.css");
    // app->useStyleSheet("/style/custom-bootstrap-theme.min.css");

    // Using Bootstrap CSS version 5.
    auto bootstrapTheme = std::make_shared<Wt::WBootstrap5Theme>();
    app->setTheme(bootstrapTheme);
    // app->setCssTheme("polished");
    app->styleSheet().addRule(".Wt-suggest b", "color: black;");

    // app->useStyleSheet("/resources/themes/bootstrap/5/css/bootstrap.css");
    // app->useStyleSheet("/resources/themes/bootstrap/5/css/bootstrap.min.css");


    // Sets style classes to the <body> element.
    // Wt::log("info") << "set body class";
    // app->setBodyClass("d-flex h-100 text-bg-dark");
    // app->setBodyClass("d-flex");
    // app->setHtmlClass("d-flex h-100 text-bg-dark");


    /**
     * Additional stylesheet.
     **/
    app->useStyleSheet("/style/trokam_common.css");

    if(page_style == "light")
    {
        app->useStyleSheet("/style/trokam_light.css");
    }
    else
    {
        app->useStyleSheet("/style/trokam_dark.css");
    }
    // app->useStyleSheet("/style/w3.css");

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

    app->root()->addWidget(std::make_unique<Trokam::SearchPage>(commonResources, app.get()));

    /**
     * Set web site title.
     **/
    app->setTitle("Trokam");

    return app;
}
