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

/// Trokam
#include "ackWidget.h"
#include "deferredWidget.h"

Trokam::AckWidget::AckWidget(boost::shared_ptr<Trokam::SharedResources> &sr,
                             Wt::WApplication* app): PageWidget(sr, app)
{
    addText(Wt::WString::tr("ack-intro"), this);
}

void Trokam::AckWidget::populateSubMenu(Wt::WMenu *menu)
{
    menu->addItem(Wt::WString::tr("donors"),       donors())->setPathComponent("");
    menu->addItem(Wt::WString::tr("developers"),   deferCreate(std::bind(&Trokam::AckWidget::developers,  this)));
    /// menu->addItem(Wt::WString::tr("translators"),  deferCreate(std::bind(&Trokam::AckWidget::translators, this)));
    menu->addItem(Wt::WString::tr("projects"),     deferCreate(std::bind(&Trokam::AckWidget::projects,    this)));
}

std::unique_ptr<Wt::WWidget> Trokam::AckWidget::donors()
{
    auto result = std::make_unique<Wt::WTemplate>(Wt::WString::tr("body-ack-donors"));
    return std::move(result);
}

std::unique_ptr<Wt::WWidget> Trokam::AckWidget::developers()
{
    auto result = std::make_unique<Wt::WTemplate>(Wt::WString::tr("body-ack-developers"));
    return std::move(result);
}

std::unique_ptr<Wt::WWidget> Trokam::AckWidget::translators()
{
    auto result = std::make_unique<Wt::WTemplate>(Wt::WString::tr("body-ack-translators"));
    return std::move(result);
}

std::unique_ptr<Wt::WWidget> Trokam::AckWidget::projects()
{
    auto result = std::make_unique<Wt::WTemplate>(Wt::WString::tr("body-ack-projects"));
    return std::move(result);
}
