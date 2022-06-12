/***********************************************************************
 *                            T R O K A M
 *                       Internet Search Engine
 *
 * Copyright (C) 2017, Nicolas Slusarenko
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

#ifndef TROKAM_PAGE_WIDGET_H
#define TROKAM_PAGE_WIDGET_H

/// C++
#include <string>

/// Boost
#include <boost/shared_ptr.hpp>

/// Wt
#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WMenu.h>
#include <Wt/WString.h>

/// Trokam
#include "sharedResources.h"

namespace Trokam
{
    class PageWidget : public Wt::WContainerWidget
    {
        public:

            PageWidget(boost::shared_ptr<Trokam::SharedResources> &sr, Wt::WApplication* app);

            virtual void populateSubMenu(Wt::WMenu *menu);

        protected:

            Wt::WApplication* application;
            boost::shared_ptr<Trokam::SharedResources> resources;

            static Wt::WText *addText(const Wt::WString& s, Wt::WContainerWidget *parent);

    };
}

#endif /// TROKAM_PAGE_WIDGET_H
