/***********************************************************************
 *                            T R O K A M
 *                       Internet Search Engine
 *                       trokam.com / trokam.org
 *
 * Copyright (C) Nicolas Slusarenko
 *               nicolas.slusarenko@trokam.com
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

// Wt
#include <Wt/WLogger.h>

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
        case Trokam::DUTCH:      return "dutch";
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

Trokam::Preferences::Preferences()
{}

Trokam::Preferences::Preferences(const std::string &input)
{
    generate(input);
}

unsigned int Trokam::Preferences::getTheme()
{
    return pref_theme;
}

void Trokam::Preferences::setTheme(const unsigned int &theme)
{
    pref_theme = theme;
}

std::vector<std::pair<int, bool>> Trokam::Preferences::getLanguages()
{
    std::vector<std::pair<int, bool>> result;
    for(unsigned int i=0; i<LANGUAGES_TOTAL; i++)
    {
        bool in_use = is_in_use(i, pref_languages);
        auto language_item = std::make_pair(i, in_use);
        result.push_back(language_item);
    }
    return result;
}

void Trokam::Preferences::setLanguages(
    const std::vector<std::pair<int, bool>> &languages)
{
    pref_languages.clear();
    for(unsigned int i=0; i<languages.size(); i++)
    {
        bool in_use = std::get<bool>(languages[i]);
        if(in_use)
        {
            pref_languages.push_back(i);
        }
    }
}

void Trokam::Preferences::generate(const std::string &input)
{
    nlohmann::json preferences;

    try
    {
        preferences = nlohmann::json::parse(input);
    }
    catch (nlohmann::json::parse_error& ex)
    {
        Wt::log("info") << "constructor: parse error at byte " << ex.byte;
    }

    Wt::log("info") << "constructor: preferences[\"theme\"]=" << preferences["theme"];

    try
    {
        pref_theme = preferences["theme"];
    }
    catch(...)
    {
        Wt::log("info") << "constructor: theme is not defined in the preferences. ----";
    }

    try
    {
        pref_languages = preferences["languages"].get<std::vector<unsigned int>>();
    }
    catch(...)
    {
        Wt::log("info") << "constructor: languages is not defined in the preferences. ----";
    }
}

std::string Trokam::Preferences::serialize()
{
    nlohmann::json preferences;
    preferences["theme"] = pref_theme;
    preferences["languages"] = pref_languages;
    std::string result = preferences.dump();
    Wt::log("info") << "serialize -- result=" << result;
    return result;
}

bool Trokam::Preferences::is_in_use(
    unsigned int &index,
    std::vector<unsigned int> &bunch)
{
    for(unsigned int i=0; i<bunch.size(); i++)
    {
        if(index == bunch[i])
        {
            return true;
        }
    }
    return false;
}