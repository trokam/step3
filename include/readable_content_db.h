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
    struct DocData
    {
        Xapian::MSetIterator it;
        float relevance = 1.0;
    };

    struct Finding
    {
        std::string title;
        std::string url;
        std::string snippet;
        float relevance_body  = 0.0;
        float relevance_url   = 0.0;
        float relevance_title = 0.0;
        float relevance_total = 0.0;
    };

    class ReadableContentDB
    {
        public:

            void open(const std::string &path);
            void add(const std::string &path);

            std::vector<Finding>
                search(
                    const std::string &querystring,
                    const std::vector<std::string> &languages,
                    Xapian::doccount results_requested = 1);

            std::vector<std::pair<std::string, Xapian::doccount>>
                lookUp(
                    const std::string &prefix);

            void close();

        private:

            const int SLOT_URL       = 0;
            const int SLOT_TITLE     = 1;
            const int SLOT_RELEVANCE = 2;

            std::unique_ptr<Xapian::Database> db;
    };
}
