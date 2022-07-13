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

// Trokam
#include "preferences.h"

std::string Trokam::Preferences::languageName(
    const unsigned int language_id)
{
    switch(language_id)
    {
        case Trokam::ENGLISH:    return "english";
        case Trokam::RUSSIAN:    return "russian";
        case Trokam::SPANISH:    return "spanish";
        case Trokam::TURKISH:    return "turkish";
        case Trokam::GERMAN:     return "german";
        case Trokam::FRENCH:     return "french";
        case Trokam::PERSIAN:    return "persian";
        case Trokam::JAPANESE:   return "japanese";
        case Trokam::VIETNAMESE: return "vietnamese";
        case Trokam::CHINESE:    return "chinese";
        case Trokam::ITALIAN:    return "italian";
        case Trokam::DUTCH:      return "ducth";
        case Trokam::ARABIC:     return "arabic";
        case Trokam::POLISH:     return "polish";
        case Trokam::PORTUGUESE: return "postuguese";
        case Trokam::INDONESIAN: return "indonesian";
        case Trokam::KOREAN:     return "korean";
        case Trokam::UKRAINIAN:  return "ukrainian";
        case Trokam::THAI:       return "thai";
        case Trokam::HEBREW:     return "hebrew";
        case Trokam::SWEDISH:    return "swedish";
    }

    // Reaching here is centainly an error.
    return "";
}
