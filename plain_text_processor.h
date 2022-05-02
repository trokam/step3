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
#include <iostream>
#include <string>
#include <vector>

// Boost
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>

// Trokam
#include "common.h"

namespace Trokam
{
    class PlainTextProcessor
    {
        public:

            static std::string get_url_prefix(
                const std::string &url);

            static void extract_url(
                const int &max_url_extracted,
                const web_doc *doc,
                std::vector<std::string> &internal,
                std::vector<std::string> &external);

            static std::string format_url(
                const std::string &url_prefix,
                std::string &url);

            static std::string getTitle(
                    const std::string &raw);

            static void extractPlainText(
                const size_t &limit,
                const std::string &raw,
                std::string &text);

            /**
             * Suitable only for small strings, since this
             * method makes a copy of the incomming text.
             */
            static std::vector<std::string> tokenize(
                std::string text);

            static std::string snippet(
                const std::string &block_text,
                const std::string &search_text,
                const size_t &snippet_length);

            static size_t case_insensitive_find(
                const std::string &text_block,
                std::string text_piece);

            static float how_much_of(
                std::string text_block,
                std::string text_piece);
    };
}
