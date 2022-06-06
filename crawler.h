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
#include <errno.h>
#include <stdlib.h>
#include <string.h>

// C++
#include <random>

// Libcurl
#include <curl/curl.h>

// Trokam
#include "common.h"
#include "warehouse.h"

namespace Trokam
{
    class Crawler
    {
        public:
            void run();

            // Initialise the crawler database.
            void initialise(
                const std::string &filename);

            void clean();

        private:
            // number of simultaneous transfers
            const size_t MAX_PARALLEL  = 7;    

            // number of URL processed per run 
            const int    TOTAL_PER_RUN = 1000;  

            // max number to URLs extracted from a web page
            const int    MAX_URL_EXTRACTED = 300;

            // max number of internal URLs to save
            const int    MAX_INTERNAL =  10;

            // max number of external URLs to save
            const int    MAX_EXTERNAL = 10;

            // The database that keeps the URLs and their
            // state: pending, downloaded, etc.
            Warehouse house;

            void setupDownload(
                CURLM *curl_multi_handler,
                const std::string &url,
                const int &id);

            // Extract and save the URLs in the document.
            void extractSaveUrl(
                const web_doc *doc);

            std::vector<std::string> getSelection(
                const std::size_t maximum,
                const std::vector<std::string> &links);
    };
}
