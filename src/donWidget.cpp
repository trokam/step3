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
#include <Wt/WMenu.h>
#include <Wt/WTemplate.h>
#include <Wt/WText.h>

/// Trokam
#include "donWidget.h"
#include "deferredWidget.h"

Trokam::DonWidget::DonWidget(boost::shared_ptr<Trokam::SharedResources> &sr,
                             Wt::WApplication* app): PageWidget(sr, app)
{
    addText(Wt::WString::tr("donation-intro"), this);
}

void Trokam::DonWidget::populateSubMenu(Wt::WMenu *menu)
{
    menu->addItem(Wt::WString::tr("whydonate"), deferCreate(std::bind(&Trokam::DonWidget::whyDonate, this)));
    menu->addItem(Wt::WString::tr("make"),      donate())->setPathComponent("");
}

std::unique_ptr<Wt::WWidget> Trokam::DonWidget::whyDonate()
{
    auto result = std::make_unique<Wt::WTemplate>(Wt::WString::tr("body-don-why"));
    return std::move(result);
}

std::unique_ptr<Wt::WWidget> Trokam::DonWidget::donate()
{
    auto result = std::make_unique<Wt::WTemplate>(Wt::WString::tr("body-don-make"));
    return std::move(result);
}
