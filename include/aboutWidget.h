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

#ifndef TROKAM_ABOUT_WIDGET_H
#define TROKAM_ABOUT_WIDGET_H

/// Trokam
#include "pageWidget.h"

namespace Trokam
{
    class AboutWidget: public Trokam::PageWidget
    {
        public:

            AboutWidget(boost::shared_ptr<Trokam::SharedResources> &sr,
                        Wt::WApplication* app);

            void populateSubMenu(Wt::WMenu *menu);

        private:

            std::unique_ptr<WWidget> basic();
            std::unique_ptr<WWidget> qa();
            std::unique_ptr<WWidget> contact();
   };
}

#endif /// TROKAM_ABOUT_WIDGET_H
