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

// C++
#include <memory>
#include <string>

// Xapian
#include <xapian.h>

// Trokam
#include "options.h"
#include "warehouse.h"

namespace Trokam
{
    class WritableContentDB
    {
        public:

            WritableContentDB(
                Trokam::Options &opt);

            void insert(
                const int &id,
                const std::string &url,
                const std::string &title,
                const std::string &text,
                const std::string &language);

            void clean();

        private:

            const int SLOT_URL   = 0;
            const int SLOT_TITLE = 1;
            const int SLOT_RELEVANCE = 2;

            Trokam::Options &options;

            std::unique_ptr<Xapian::WritableDatabase> db;

            std::string db_path;
    };
}
