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
#include "grasp.h"
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

        Trokam::Grasp grasp;
        grasp.clean();
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
        Trokam::Crawler crawler;
        for(int i=0; i<10; i++)
        {
            crawler.run();
        }
    }
    else if(action == "search")
    {
        // Indexing the web.
        std::cout << "Searching the database ...\n";
        std::string terms = opt.terms();
        if(terms == "")
        {
            std::cerr << "Provide a seeds file using --seeds-file\n";
            exit(1);
        }
        Trokam::Grasp grasp;
        grasp.search(terms);
    }
    else
    {
        std::cerr << "Action '" << action << "' is not valid.\n";
    }

    std::cout << "\n---------- Bye! ----------\n";        
    return EXIT_SUCCESS;
}
