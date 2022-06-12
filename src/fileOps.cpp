/***********************************************************************
 *                            T R O K A M
 *                       Internet Search Engine
 *
 * Copyright (C) 2018, Nicolas Slusarenko
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

/// C++
#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <array>

/// Boost
#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>

/// Magic
#include <magic.h>

/// Trokam
#include "common.h"
#include "fileOps.h"
// #include "textProcessing.h"

std::string Trokam::FileOps::read(const std::string &filename)
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

void Trokam::FileOps::read(const std::string &filename,
                                 std::string &content)
{
    std::ifstream inputFile(filename.c_str(), std::ios::in | std::ios::binary);
    if(inputFile)
    {
        inputFile.seekg(0, std::ios::end);
        content.resize(inputFile.tellg());
        inputFile.seekg(0, std::ios::beg);
        inputFile.read(&content[0], content.size());
        inputFile.close();
    }
}

void Trokam::FileOps::read(const std::string &filename,
                                 std::vector<std::vector<std::string>> &content)
{
    std::ifstream inputFile(filename.c_str(), std::ios::in | std::ios::binary);

    std::string line;
    while (std::getline(inputFile, line))
    {
        boost::algorithm::trim_if(line, boost::algorithm::is_any_of(" \t\n\r\""));

        if ((line != "") && (line[0] != '#'))
        {
            std::vector<std::string> row;

            typedef boost::char_separator<char> char_sep;
            typedef boost::tokenizer<char_sep> tokenizer;

            boost::char_separator<char> sep(":");
            tokenizer tok(line, sep);

            for(tokenizer::iterator it= tok.begin(); it!=tok.end(); ++it)
            {
                const std::string partial= *it;
                row.push_back(partial);
            }

            content.push_back(row);
        }
    }
}

bool Trokam::FileOps::isEmpty(const std::string &filename)
{
    std::ifstream inputFile(filename.c_str());
    bool state= (inputFile.peek() == std::ifstream::traits_type::eof());
    return state;
}

void Trokam::FileOps::save(const std::string &filename,
                           const std::string &content)
{
    std::ofstream out(filename.c_str());
    out << content;
    out.close();
}

void Trokam::FileOps::rmFile(const std::string &filename)
{
    std::string command= "rm -f " + filename;
    const int status= system(command.c_str());
    if(status != 0)
    {
        std::cerr << "failure on deleting file: " << filename << "\n";
    }
}

void Trokam::FileOps::rmDir(const std::string &dirname)
{
    std::string command= "rm -rf " +  dirname;
    const int status= system(command.c_str());
    if(status != 0)
    {
        std::cerr << "failure on deleting directory: " << dirname << "\n";
    }
}

void Trokam::FileOps::mkDir(const std::string &dirname)
{
    std::string command= "mkdir -p " +  dirname;
    const int status= system(command.c_str());
    if(status != 0)
    {
        std::cerr << "failure on creating directory: " << dirname << "\n";
    }
}

/**
std::string Trokam::FileOps::type(const std::string &file)
{
    magic_t mg= magic_open(MAGIC_CONTINUE|MAGIC_ERROR|MAGIC_MIME);
    magic_load(mg, NULL);
    std::string type(magic_file(mg, file.c_str()));
    magic_close(mg);

    return type;
}
**/

std::string Trokam::FileOps::exec(const std::string &command)
{
    std::array<char, 128> buffer;
    std::string result;
    std::shared_ptr<FILE> pipe(popen(command.c_str(), "r"), pclose);

    if(!pipe)
    {
        throw std::runtime_error("popen() failed!");
    }

    while(!feof(pipe.get()))
    {
        if(fgets(buffer.data(), 128, pipe.get()) != nullptr)
        {
            result += buffer.data();
        }
    }

    return result;
}

/**
void Trokam::FileOps::getFileSnippet(const std::string &contentDir,
                                     const std::string &terms,
                                     const int &pageIndex,
                                     std::string &snippet)
{
    std::string directory;
    std::string file;
    Trokam::FileOps::getDirFile(contentDir, pageIndex, directory, file);

    std::cout << "filename: " << file << "\n";

    std::string content;
    Trokam::FileOps::read(file, content);

    std::size_t loc= content.find(terms);
    if(loc != std::string::npos)
    {
        std::cout << "terms found at: " << loc << "\n";

        std::size_t ini= 0;
        std::size_t count= 400;

        if(loc > 200)
        {
            ini = loc - 200;
        }

        snippet = content.substr(ini, count);
        const std::string strongText= "<strong>" + terms + "</strong>";
        boost::replace_all(snippet, terms, strongText);
    }
    else
    {
        std::cout << "terms not found\n";
        snippet = content.substr(0, 400);
    }

    boost::replace_all(snippet, "___________", "");
    boost::replace_all(snippet, "_____", "");
    boost::replace_all(snippet, "__", "");
}
**/

/**
void Trokam::FileOps::getDirFile(const std::string &contentDir,
                                 const int &index,
                                 std::string &directory,
                                 std::string &file)
{
    const std::string locator= Trokam::TextProcessing::leftPadding(std::to_string(index), 12, "0");
    const std::string dir1= locator.substr(0, 4);
    const std::string dir2= locator.substr(4, 4);
    const std::string filename= locator.substr(8, 4);

    directory= contentDir + SLASH + dir1 + SLASH + dir2;
    file= directory + SLASH + filename;
}
**/
