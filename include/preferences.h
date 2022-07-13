/***********************************************************************
 *                            T R O K A M
 *                       Internet Search Engine
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

#pragma once

// C++
#include <string>

/**
 * Language numbering loosely correlates
 * language frequency of Internet pages.
 **/

namespace Trokam
{
    const unsigned int LANGUAGES_TOTAL = 21;

    enum Language: unsigned int
    {
        ENGLISH    =  0,
        RUSSIAN    =  1,
        SPANISH    =  2,
        TURKISH    =  3,
        GERMAN     =  4,
        FRENCH     =  5,
        PERSIAN    =  6,
        JAPANESE   =  7,
        VIETNAMESE =  8,
        CHINESE    =  9,
        ITALIAN    = 10,
        DUTCH      = 11,
        ARABIC     = 12,
        POLISH     = 13,
        PORTUGUESE = 14,
        INDONESIAN = 15,
        KOREAN     = 16,
        UKRAINIAN  = 17,
        THAI       = 18,
        HEBREW     = 19,
        SWEDISH    = 20
    };

    class Preferences
    {
        public:
            static std::string languageName(
                const unsigned int language_id);
    };
}
