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
#include <Wt/WTemplate.h>

/// Trokam
#include "aboutWidget.h"
#include "deferredWidget.h"

Trokam::AboutWidget::AboutWidget(boost::shared_ptr<Trokam::SharedResources> &sr,
                                 Wt::WApplication* app): PageWidget(sr, app)
{
    addText(Wt::WString::tr("about-intro"), this);
}

void Trokam::AboutWidget::populateSubMenu(Wt::WMenu *menu)
{
    menu->addItem(Wt::WString::tr("menu-about-basic"), basic())->setPathComponent("");
    /// menu->addItem(Wt::WString::tr("menu-about-qa"),    deferCreate(std::bind(&Trokam::AboutWidget::qa,  this)));
    menu->addItem(Wt::WString::tr("menu-about-contact"),  deferCreate(std::bind(&Trokam::AboutWidget::contact,  this)));
}

std::unique_ptr<Wt::WWidget> Trokam::AboutWidget::basic()
{
    auto result = std::make_unique<Wt::WTemplate>(Wt::WString::tr("body-about-basic"));
    return std::move(result);
}


std::unique_ptr<Wt::WWidget> Trokam::AboutWidget::qa()
{
    auto result = std::make_unique<Wt::WTemplate>(Wt::WString::tr("body-about-qa"));
    return std::move(result);
}

std::unique_ptr<Wt::WWidget> Trokam::AboutWidget::contact()
{
    auto result = std::make_unique<Wt::WTemplate>(Wt::WString::tr("body-about-contact"));
    return std::move(result);
}
