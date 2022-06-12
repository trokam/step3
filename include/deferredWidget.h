/***********************************************************************
 *                            T R O K A M
 *                       Internet Search Engine
 *
 * Copyright (C) 2018, Nicolas Slusarenko
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

#ifndef TROKAM_DEFERRED_WIDGET_H
#define TROKAM_DEFERRED_WIDGET_H

/// Wt
#include <Wt/WContainerWidget.h>

/**
 * A utility container widget which defers creation of its single
 * child widget until the container is loaded (which is done on-demand
 * by a WMenu). The constructor takes the create function for the
 * widget as a parameter.
 *
 * We use this to defer widget creation until needed, which also defers
 * loading auxiliary javascript libraries.
 **/

template <typename Function>
class DeferredWidget: public Wt::WContainerWidget
{
    public:

        DeferredWidget(Function f): f_(f)
        {}

    private:

        void load()
        {
            addWidget(f_());
            Wt::WContainerWidget::load();
        }

        Function f_;
};

template <typename Function>
std::unique_ptr<DeferredWidget<Function>> deferCreate(Function f)
{
    return std::make_unique<DeferredWidget<Function>>(f);
}

#endif /// TROKAM_DEFERRED_WIDGET_H
