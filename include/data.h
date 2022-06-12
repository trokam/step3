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

#ifndef TROKAM_DATA_H
#define TROKAM_DATA_H

/// C++
#include <string>

namespace Trokam
{
    struct Sequence
    {
        int index;
        std::string value;
        int occurrences;
    };

    struct Page
    {
        int index;
        std::string url;
        int relevance;
    };

    struct PairTN
    {
        std::string text;
        int number;
    };

    struct Findings
    {
        int pageIndex;
        std::string url;
        std::string title;
        int relevanceInBody;
        int relevanceInUrl;
        int relevanceInTitle;
        int relevanceTotal;
        std::string snippet;
        std::string phrase;
        int phraseMatching;
        int dbId;
    };
}

#endif  /// TROKAM_DATA_H
