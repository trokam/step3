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
#include <iostream>

// Boost
#include <boost/program_options.hpp>

// Trokam
#include "options.h"

Trokam::Options::Options()
{}

Trokam::Options::Options(int argc, char* argv[])
{
    // Program Options.
    try
    {
        boost::program_options::options_description desc("Allowed options");
        desc.add_options()
            ("help",         "Produce help message")
            ("action",       boost::program_options::value<std::string>(),  "Possible actions: clean, init, index, look-up and search.")
            ("seeds-file",   boost::program_options::value<std::string>(),  "File with first URLs.")
            ("terms",        boost::program_options::value<std::string>(),  "Terms to search.")
            ("languages",    boost::program_options::value<std::string>(),  "Languages to search. Comma separated.")
            ("db-content",   boost::program_options::value<std::string>(),  "Path to the content database.")
            ("offset",       boost::program_options::value<unsigned int>(), "Results page.")
            ("pagesize",     boost::program_options::value<unsigned int>(), "Results per page.")
            ("max-results",  boost::program_options::value<unsigned int>(), "Maximum number of results.")
            ("cycles",       boost::program_options::value<unsigned int>(), "Indexing cycles.");
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

        if(vm.count("db-content"))
        {
            opt_db_content = vm["db-content"].as<std::string>();
        }

        if(vm.count("offset"))
        {
            opt_offset = vm["offset"].as<unsigned int>();
        }

        if(vm.count("pagesize"))
        {
            opt_page_size = vm["pagesize"].as<unsigned int>();
        }

        if(vm.count("max-results"))
        {
            opt_max_results = vm["max-results"].as<unsigned int>();
        }

        if(vm.count("cycles"))
        {
            opt_cycles = vm["cycles"].as<unsigned int>();
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

std::string Trokam::Options::db_content() const
{
    return opt_db_content;
}

unsigned int Trokam::Options::offset() const
{
    return opt_offset;
}

unsigned int Trokam::Options::pageSize() const
{
    return opt_page_size;
}

unsigned int Trokam::Options::maxResults() const
{
    return opt_max_results;
}

unsigned int Trokam::Options::cycles() const
{
    return opt_cycles;
}
