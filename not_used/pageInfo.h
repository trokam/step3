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

#ifndef TROKAM_PAGEINFO_H
#define TROKAM_PAGEINFO_H

/// C++
#include <string>

/// Trokam
// #include "differentStrings.h"
#include "textStore.h"

namespace Trokam
{
    class PageInfo
    {
        public:

            PageInfo();
            PageInfo(const PageInfo &value);
            PageInfo& operator= (const PageInfo &value);

            int index;
            std::string urlOrigin;
            std::string urlForwarded;
            int domainIndex;
            int domainType;
            int pageType;
            std::string raw;
            std::string content;
            std::string links;
            std::string title;
            std::string type;
            int size;
            float complexity;
            Trokam::TextStore sequences;
            // Trokam::DifferentStrings urlBag;
    };
}

#endif  /// TROKAM_PAGEINFO_H
