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

// C++
#include <iostream>

// Trokam
#include "crawler.h"
#include "readable_content_db.h"
#include "writable_content_db.h"
#include "options.h"

int main(int argc, char *argv[])
{
    Trokam::Options opt(argc, argv);
    const std::string action = opt.action();

    if(action == "clean")
    {
        // Initializing the crawler database.
        std::cout << "Clearing crawler database ...\n";
        Trokam::Crawler crawler;
        crawler.clean();

        Trokam::WritableContentDB writable_content_db;
        writable_content_db.clean();
    }
    else if(action == "init")
    {
        // Initializing the crawler database.
        std::cout << "Initializing crawler database ...\n";
        std::string seeds_file = opt.seedsFile();
        if(seeds_file == "")
        {
            std::cerr << "Provide a seeds file using --seeds-file\n";
            exit(1);
        }

        Trokam::Crawler crawler;
        crawler.initialise(seeds_file);
    }
    else if(action == "index")
    {
        // Indexing the web.
        std::cout << "Indexind the web ...\n";
        const int TOTAL_RUNS = 1;
        Trokam::Crawler crawler;
        for(int i=0; i<TOTAL_RUNS; i++)
        {
            std::cout
                << "---------- run start:"
                << i << " ----------\n";
            crawler.run();
        }
    }
    else if(action == "search")
    {
        // Searching the document database.
        std::cout << "Searching the database ...\n\n";
        std::string terms = opt.terms();
        if(terms == "")
        {
            std::cerr << "Provide terms to search using --terms\n";
            exit(1);
        }
        std::string languages = opt.languages();
        unsigned int offset = opt.offset();
        unsigned int pagesize = opt.pagesize();
        Trokam::ReadableContentDB readable_content_db;
        readable_content_db.search(terms, languages, offset, pagesize);
    }
    else
    {
        std::cerr << "Action '" << action << "' is not valid.\n";
    }

    std::cout << "\n---------- Bye! ----------\n";
    return EXIT_SUCCESS;
}
