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

#pragma once

// C
#include <limits.h>

// C++
#include <string>

namespace Trokam
{
    class Options
    {
        public:

            Options();
            Options(int argc, char* argv[]);
            std::string action() const;
            std::string seedsFile() const;
            std::string terms() const;
            std::string languages() const;
            std::string db_content() const;
            unsigned int offset() const;
            unsigned int pageSize() const;
            unsigned maxResults() const;

        private:

            std::string opt_action;
            std::string opt_seeds_file;
            std::string opt_terms;
            std::string opt_languages;
            std::string opt_db_content;
            unsigned int opt_offset = 1;
            unsigned int opt_page_size = 5;
            unsigned int opt_max_results = UINT_MAX;
    };
}
