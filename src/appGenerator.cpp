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

/// Wt
#include <Wt/WApplication.h>
#include <Wt/WBootstrapTheme.h>
#include <Wt/WEnvironment.h>
#include <Wt/WHBoxLayout.h>
#include <Wt/WBootstrapTheme.h>
#include <Wt/WCssTheme.h>
#include <Wt/WLink.h>

/// Trokam
#include "common.h"
#include "appGenerator.h"
#include "topWindow.h"

Trokam::AppGenerator::AppGenerator(Trokam::Options &opt): commonResources(new Trokam::SharedResources(opt))
{}

std::unique_ptr<Wt::WApplication> Trokam::AppGenerator::createApplication(const Wt::WEnvironment& env)
{
    /**
     * Instantiate the application. object.
     **/
    auto app = Wt::cpp14::make_unique<Wt::WApplication>(env);

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
    auto bootstrapTheme = std::make_shared<Wt::WBootstrapTheme>();
    bootstrapTheme->setVersion(Wt::BootstrapVersion::v3);
    bootstrapTheme->setResponsive(true);
    app->setTheme(bootstrapTheme);
    app->useStyleSheet("resources/themes/bootstrap/3/bootstrap-theme.min.css");

    /**
     * Customized Bootstrap 3 CCS.
     **/
    app->useStyleSheet("/style/custom-bootstrap.css");
    app->useStyleSheet("/style/custom-bootstrap.min.css");
    app->useStyleSheet("/style/custom-bootstrap-theme.css");
    app->useStyleSheet("/style/custom-bootstrap-theme.min.css");

    /**
     * Additional stylesheet.
     **/
    app->useStyleSheet("/style/trokam.css");
    app->useStyleSheet("/style/layout.css");

    /**
     * Load text bundles.
     **/
    app->messageResourceBundle().use(app->appRoot() + "report");
    app->messageResourceBundle().use(app->appRoot() + "text");
    app->messageResourceBundle().use(app->appRoot() + "src");

    /**
     * Add the only one widget in the application layout.
     **/
    auto layout = app->root()->setLayout(Wt::cpp14::make_unique<Wt::WHBoxLayout>());
    layout->setPreferredImplementation(Wt::LayoutImplementation::JavaScript);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(Wt::cpp14::make_unique<Trokam::TopWindow>(commonResources, app.get()));

    /**
     * Set web site title.
     **/
    app->setTitle("Trokam Search Engine");

    return app;
}
