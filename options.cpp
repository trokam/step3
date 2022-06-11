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
#include <string>
#include <iostream>

// Boost
#include <boost/program_options.hpp>

// Trokam
#include "options.h"

Trokam::Options::Options(int argc, char* argv[])
{
    // Program Options.
    try
    {
        boost::program_options::options_description desc("Allowed options");
        desc.add_options()
            ("help",       "Produce help message")
            ("action",     boost::program_options::value<std::string>(),  "Possible actions: empty, init, index and search.")
            ("seeds-file", boost::program_options::value<std::string>(),  "File with first URLs.")
            ("terms",      boost::program_options::value<std::string>(),  "Terms to search.")
            ("languages",  boost::program_options::value<std::string>(),  "Languages to search. Comma separated.")
            ("offset",     boost::program_options::value<unsigned int>(), "Results page.")
            ("pagesize",   boost::program_options::value<unsigned int>(), "Results per page.");
        boost::program_options::variables_map vm;
        boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
        boost::program_options::notify(vm);

        if(vm.count("help"))
        {
            std::cout << desc << std::endl;
            exit(0);
        }

        if(vm.count("action"))
        {
            opt_action = vm["action"].as<std::string>();
        }

        if(vm.count("seeds-file"))
        {
            opt_seeds_file = vm["seeds-file"].as<std::string>();
        }

        if(vm.count("terms"))
        {
            opt_terms = vm["terms"].as<std::string>();
        }

        if(vm.count("languages"))
        {
            opt_languages = vm["languages"].as<std::string>();
        }

        if(vm.count("offset"))
        {
            opt_offset = vm["offset"].as<unsigned int>();
        }

        if(vm.count("pagesize"))
        {
            opt_pagesize = vm["pagesize"].as<unsigned int>();
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        exit(1);
    }
}

std::string Trokam::Options::action() const
{
    return opt_action;
}

std::string Trokam::Options::seedsFile() const
{
    return opt_seeds_file;
}

std::string Trokam::Options::terms() const
{
    return opt_terms;
}

std::string Trokam::Options::languages() const
{
    return opt_languages;
}

unsigned int Trokam::Options::offset() const
{
    return opt_offset;
}

unsigned int Trokam::Options::pagesize() const
{
    return opt_pagesize;
}
