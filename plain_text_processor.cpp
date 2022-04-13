/***********************************************************************
 *                            T R O K A M
 *                       Internet Search Engine
 *
 * Copyright (C) 2022, Nicolas Slusarenko
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

// Boost
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>

// Trokam
#include "plain_text_processor.h"

std::string PlainTextProcessor::getTitle(
    const std::string &raw)
{
    std::string title = "(empty)";
    try
    {
        std::string::size_type ini= raw.find("<title") + 6;
        if (ini != std::string::npos)
        {
            ini = raw.find(">", ini) + 1;
        }

        std::string::size_type end= raw.find("</title>", ini);
        if((ini != std::string::npos) && (end != std::string::npos))
        {
            if((end-ini) < 200)
            {
                title= raw.substr(ini, end-ini);
            }
            else
            {
                title= raw.substr(ini, 200);
            }
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << __PRETTY_FUNCTION__;
        std::cerr << "error:" << e.what() << '\n';
    }

    boost::algorithm::trim_if(title, boost::algorithm::is_any_of("\n\r\t\\\""));

    return title;
}
