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
#include <fstream>
#include <iostream>

// Boost
#include <boost/algorithm/string.hpp>

// Fmt
#include <fmt/core.h>

// Trokam
#include "file_ops.h"

std::string Trokam::FileOps::read(
    const std::string &filename)
{
    std::string content;
    std::ifstream input_file(filename.c_str(), std::ios::in | std::ios::binary);
    if(input_file)
    {
        input_file.seekg(0, std::ios::end);
        content.resize(input_file.tellg());
        input_file.seekg(0, std::ios::beg);
        input_file.read(&content[0], content.size());
        input_file.close();
    }
    return content;
}

std::string Trokam::FileOps::readLines(
    const size_t SIZE_LIMIT,
    const std::string &filename)
{
    std::string result;
    std::ifstream input_file(filename);
    if (!input_file.is_open())
    {
        std::cerr << "fail: could not open file:'" << filename << "'\n";
        return "";
    }

    std::string line;
    while(
        (std::getline(input_file, line)) &&
        (result.size() < SIZE_LIMIT))
    {
        boost::algorithm::trim_if(
            line, boost::algorithm::is_any_of(" \t\n\r\""));
        result += line + ' ';
    }

    return result;
}

void Trokam::FileOps::readNoComment(
    const std::string &filename,
    std::vector<std::string> &content)
{
    std::ifstream input_file(
        filename.c_str(),
        std::ios::in | std::ios::binary);

    std::string line;
    while(std::getline(input_file, line))
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

void Trokam::FileOps::rmDir(
    const std::string &dirname)
{
    std::string command= "rm -rf " + dirname;
    const int status= system(command.c_str());
    if(status != 0)
    {
        std::cerr << "failure on deleting directory: " << dirname << "\n";
    }
}

void Trokam::FileOps::cleanDir(
    const std::string &dirname)
{
    std::string command= "rm -rf " + dirname + "/*";
    const int status= system(command.c_str());
    if(status != 0)
    {
        std::cerr << "failure on cleaning directory: " << dirname << "\n";
    }
}

std::string Trokam::FileOps::generateDirName(
    const std::string &stem,
    const int &index,
    const int &serial)
{
    std::string result =
        fmt::format("{}-{:02}-{:06}", stem, index, serial);
    return result;
}

std::string Trokam::FileOps::generateDirLabel(
    const std::string &stem,
    const int &index,
    const int &serial)
{
    std::string result =
        fmt::format("{}_{:02}_{:06}", stem, index, serial);
    return result;
}
