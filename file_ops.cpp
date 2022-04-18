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
#include <fstream>

// Boost
#include <boost/algorithm/string.hpp>

// Trokam
#include "file_ops.h"

std::string Trokam::FileOps::read(
    const std::string &filename)
{
    std::string content;
    std::ifstream inputFile(filename.c_str(), std::ios::in | std::ios::binary);
    if(inputFile)
    {
        inputFile.seekg(0, std::ios::end);
        content.resize(inputFile.tellg());
        inputFile.seekg(0, std::ios::beg);
        inputFile.read(&content[0], content.size());
        inputFile.close();
    }
    return content;
}

void Trokam::FileOps::readNoComment(
    const std::string &filename,
    std::vector<std::string> &content)
{
    std::ifstream inputFile(
        filename.c_str(),
        std::ios::in | std::ios::binary);

    std::string line;
    while(std::getline(inputFile, line))
    {
        boost::algorithm::trim_if(
            line,
            boost::algorithm::is_any_of(" \t\n\r\""));

        if ((line != "") && (line[0] != '#'))
        {
            content.push_back(line);
        }
    }
}

void Trokam::FileOps::save(
    const std::string &filename,
    const std::string &content)
{
    std::ofstream out(filename.c_str());
    out << content;
    out.close();
}
