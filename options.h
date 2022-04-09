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

/// C++
// #include <string>

namespace Trokam
{
    class Options
    {
        public:

            Options(int argc, char* argv[]);
            // bool clear() const;
            // bool init() const;
            // bool index() const;
            std::string action() const;
            std::string seedsFile() const;

        private:

            // bool opt_clear = false;
            // bool opt_init  = false;
            // bool opt_index = false;
            std::string opt_action;
            std::string opt_seeds_file;
    };
}
