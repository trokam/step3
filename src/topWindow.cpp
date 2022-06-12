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

/// Wt
#include <Wt/WApplication.h>
#include <Wt/WHBoxLayout.h>
#include <Wt/WImage.h>
#include <Wt/WMenu.h>
#include <Wt/WNavigationBar.h>
#include <Wt/WLineEdit.h>
#include <Wt/WStackedWidget.h>
#include <Wt/WString.h>
#include <Wt/WText.h>
#include <Wt/WVBoxLayout.h>

/// Trokam
#include "aboutWidget.h"
#include "searchWidget.h"
#include "ackWidget.h"
#include "donWidget.h"
#include "topWindow.h"
#include "sharedResources.h"

Trokam::TopWindow::TopWindow(boost::shared_ptr<Trokam::SharedResources> &sr,
                             Wt::WApplication* app): Wt::WContainerWidget(),
                                                     application(app)
{
    setOverflow(Wt::Overflow::Hidden);

    auto navigation = std::make_unique<Wt::WNavigationBar>();
    navigation_ = navigation.get();

    navigation_->addStyleClass("main-nav");
    navigation_->addStyleClass("navbar-inverse");
    navigation_->setTitle("Trokam", "http://trokam.com/");
    navigation_->setResponsive(true);

    auto contentsStack= std::make_unique<Wt::WStackedWidget>();
    contentsStack_ = contentsStack.get();

    Wt::WAnimation animation(Wt::AnimationEffect::Fade,
                             Wt::TimingFunction::Linear,
                             200);
    contentsStack_->setTransitionAnimation(animation, true);

    /**
     * Setup the top-level menu
     **/
    auto menu = std::make_unique<Wt::WMenu>(contentsStack_);
    menu->setInternalPathEnabled();
    menu->setInternalBasePath("/");

    /**
     * The search function is implemented with a simple layout,
     * and is inserted into the menu as the front page.
     **/
    addFrontPageToMenu(menu.get(), Wt::WString::tr("search"), std::make_unique<Trokam::SearchWidget>(sr, application));

    /**
     * Widgets meant to display information have complex layout.
     **/
    addToMenu(menu.get(), Wt::WString::tr("about"),            std::make_unique<Trokam::AboutWidget>(sr, application));
    addToMenu(menu.get(), Wt::WString::tr("acknowledgements"), std::make_unique<Trokam::AckWidget>(sr, application));
    addToMenu(menu.get(), Wt::WString::tr("donate"),           std::make_unique<Trokam::DonWidget>(sr, application));

    navigation_->addMenu(std::move(menu));

    /**
     * Add it all inside a layout
     **/
    auto layout = this->setLayout(std::make_unique<Wt::WVBoxLayout>());
    layout->setPreferredImplementation(Wt::LayoutImplementation::JavaScript);
    layout->addWidget(std::move(navigation), 0);
    layout->addWidget(std::move(contentsStack), 1);
    layout->setContentsMargins(0, 0, 0, 0);
}

Wt::WMenuItem *Trokam::TopWindow::addToMenu(Wt::WMenu *menu,
					                        const Wt::WString& name,
					                        std::unique_ptr<Trokam::PageWidget> topic)
{
    auto topic_ = topic.get();
    auto result = std::make_unique<Wt::WContainerWidget>();

    auto pane = std::make_unique<Wt::WContainerWidget>();
    auto pane_ = pane.get();

    auto vAboutWidget = result->setLayout(std::make_unique<Wt::WVBoxLayout>());
    vAboutWidget->setPreferredImplementation(Wt::LayoutImplementation::JavaScript);
    vAboutWidget->setContentsMargins(0, 0, 0, 0);
    vAboutWidget->addWidget(std::move(topic));
    vAboutWidget->addWidget(std::move(pane), 1);

    auto hAboutWidget = pane_->setLayout(std::make_unique<Wt::WHBoxLayout>());
    hAboutWidget->setPreferredImplementation(Wt::LayoutImplementation::JavaScript);

    auto item = std::make_unique<Wt::WMenuItem>(name, std::move(result));
    item->setPathComponent(name.key());
    auto item_ = menu->addItem(std::move(item));

    auto subStack = std::make_unique<Wt::WStackedWidget>();
    subStack->addStyleClass("contents");
    subStack->setOverflow(Wt::Overflow::Auto);

    Wt::WAnimation animation(Wt::AnimationEffect::Fade,
                             Wt::TimingFunction::Linear,
                             100);
    subStack->setTransitionAnimation(animation, true);

    auto subMenu = std::make_unique<Wt::WMenu>(subStack.get());
    auto subMenu_ = subMenu.get();
    subMenu_->addStyleClass("nav-pills nav-stacked submenu");
    subMenu_->setWidth(200);

    hAboutWidget->addWidget(std::move(subMenu));
    hAboutWidget->addWidget(std::move(subStack),1);

    subMenu_->setInternalPathEnabled();
    subMenu_->setInternalBasePath("/" + item_->pathComponent());

    topic_->populateSubMenu(subMenu_);

    return item_;
}

Wt::WMenuItem *Trokam::TopWindow::addFrontPageToMenu(Wt::WMenu *menu,
					                                 const Wt::WString& name,
					                                 std::unique_ptr<Trokam::PageWidget> topic)
{
    auto item = std::make_unique<Wt::WMenuItem>(name, std::move(topic));
    item->setPathComponent(name.key());
    auto item_ = menu->addItem(std::move(item));

    return item_;
}
