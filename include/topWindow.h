/***********************************************************************
 *                            T R O K A M
 *                       Internet Search Engine
 *
 * Copyright (C) 2017, Nicolas Slusarenko
 *                     nicolas.slusarenko@trokam.com
 *
 * Copyright (C) 2017, Emweb bvba, Heverlee, Belgium.
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

#ifndef TROKAM_TOP_WINDOW_H
#define TROKAM_TOP_WINDOW_H

/// C++
#include <memory>
#include <string>

/// Wt
#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WNavigationBar.h>
#include <Wt/WMessageBox.h>
#include <Wt/WMenu.h>

/// Trokam
#include "bundle.h"
#include "data.h"
#include "pageWidget.h"
#include "sharedResources.h"

namespace Trokam
{
    class TopWindow : public Wt::WContainerWidget
    {
        public:

            TopWindow(boost::shared_ptr<Trokam::SharedResources> &sr, Wt::WApplication* app);

        private:

            Wt::WApplication *application;

            Wt::WNavigationBar *navigation_;

            Wt::WStackedWidget *contentsStack_;

            Wt::WMenuItem *addToMenu(Wt::WMenu *menu,
                                     const Wt::WString& name,
                                     std::unique_ptr<Trokam::PageWidget> topic);

            Wt::WMenuItem *addFrontPageToMenu(Wt::WMenu *menu,
					                          const Wt::WString& name,
					                          std::unique_ptr<Trokam::PageWidget> topic);
    };
}

#endif /// TROKAM_TOP_WINDOW_H
