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

// C++
#include <string>

// Trokam
#include "common.h"
#include "writable_content_db.h"
#include "language_detection.h"

namespace Trokam
{
    class DocProcessor
    {
        public:

            DocProcessor(Trokam::Options &opt);

            void insert(
                const web_doc *doc,
                const std::string &retrieval_error,
                const int &set_for_download);

            void show(
                const web_doc *doc,
                const int &correlative,
                const int &status,
                const std::string &retrieval_error,
                const std::string &title,
                const std::string &language);

        private:

            const size_t TEXT_LENGTH_LIMIT = 15000;

            Trokam::Options &options;

            std::string text;
            std::string title;

            Trokam::WritableContentDB writable_content_db;
            Trokam::LanguageDetection ld;

            int extractPlainText();

            void extractTitle();
    };
}
