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

/// C++
#include <sstream>

/// Wt
#include <Wt/WText.h>

/// Trokam
#include "pageWidget.h"

Trokam::PageWidget::PageWidget(boost::shared_ptr<Trokam::SharedResources> &sr,
                               Wt::WApplication* app): WContainerWidget(),
                                                       application(app)
{
    resources= sr;
}

void Trokam::PageWidget::populateSubMenu(Wt::WMenu *menu)
{}

Wt::WText *Trokam::PageWidget::addText(const Wt::WString& s, Wt::WContainerWidget *parent)
{
    auto text = parent->addWidget(std::make_unique<Wt::WText>(s));

    bool literal = s.literal();
    if (!literal)
    {
        text->setInternalPathEncoding(true);
    }

    return text;
}
