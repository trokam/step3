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

// Json
#include <nlohmann/json.hpp>

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

    enum Theme: unsigned int
    {
        LIGHT =  0,
        DARK  =  1,
    };

    class Preferences
    {
        public:

            Preferences();
            Preferences(const std::string &raw);

            // unsigned int languages();
            unsigned int getTheme();
            void setTheme(const unsigned int &theme);

            // std::vector<unsigned int> getLanguages();
            // void setLanguages(const std::vector<unsigned int> &languages);

            std::vector<std::pair<int, bool>> getLanguages();
            void setLanguages(const std::vector<std::pair<int, bool>> &languages);

            void generate(const std::string &input);
            std::string serialize();

            static std::string languageName(
                const unsigned int language_id);

        private:

            // nlohmann::json preferences;
            unsigned int pref_theme = Trokam::Theme::LIGHT;
            bool pref_show_analysis = true;
            std::vector<unsigned int> pref_languages = {Trokam::Language::ENGLISH};

            bool is_in_use(
                unsigned int &index,
                std::vector<unsigned int> &bunch);
    };
}
