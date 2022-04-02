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

// Libcurl
#include <curl/curl.h>

// Trokam
#include "warehouse.h"

namespace Trokam
{
    class Crawler
    {
        public:
            void run();

            void setup_download(
                CURLM *curl_multi_handler,
                const std::string &url,
                const int &id);

        private:
            // number of simultaneous transfers
            const size_t MAX_PARALLEL  = 7;    

            // number of URL processed per run 
            const int    TOTAL_PER_RUN = 500;  
    };
}
