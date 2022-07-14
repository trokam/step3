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

// Boost
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>

// Trokam
#include "plain_text_processor.h"

std::string Trokam::PlainTextProcessor::getUrlPrefix(
    const std::string &url)
{   std::string result;

    std::string::size_type ini= url.find("https://");
    if(ini != std::string::npos)
    {
        std::string::size_type end= url.find("/", ini+8);
        if(end != std::string::npos)
        {
            result= url.substr(ini, end-ini);
        }
        else
        {
            result= url.substr(ini, end-ini);
        }
    }

    ini= url.find("http://");
    if(ini != std::string::npos)
    {
        std::string::size_type end= url.find("/", ini+7);
        if(end != std::string::npos)
        {
            result= url.substr(ini, end-ini);
        }
        else
        {
            result= url.substr(ini, end-ini);
        }
    }

    return result;
}

void Trokam::PlainTextProcessor::extractUrl(
    const int &max_url_extracted,
    const web_doc *doc,
    std::vector<std::string> &internal,
    std::vector<std::string> &external)
{
    boost::regex e(
        "<\\s*A\\s+[^>]*href\\s*=\\s*\"([^\"]*)\"",
        boost::regex::normal | boost::regbase::icase);

    int count= 0;
    std::string url_prefix = PlainTextProcessor::getUrlPrefix(doc->url);
    boost::sregex_token_iterator i(doc->raw.begin(), doc->raw.end(), e, 1);
    boost::sregex_token_iterator j;
    while((i != j) && (count<max_url_extracted))
    {
        std::string url= *i;
        url = formatUrl(url_prefix, url);

        if(!url.empty())
        {
            if(url.substr(0, url_prefix.length()) == url_prefix)
            {
                internal.push_back(url);
            }
            else
            {
                external.push_back(url);
            }
        }
        i++;
        count++;
    }
}

std::string Trokam::PlainTextProcessor::formatUrl(
    const std::string &url_prefix,
    std::string &url)
{
    try
    {
        // Discard some pieces of text if they have strage content
        // of if they are too long.
        if(url.find("#") != std::string::npos)
        {
            url.clear();
        }
        if(url.find("'") != std::string::npos)
        {
            url.clear();
        }
        if(url.find("\n") != std::string::npos)
        {
            url.clear();
        }
        if(url.find("action=edit") != std::string::npos)
        {
            url.clear();
        }
        if(url.find("javascript:void(0)") != std::string::npos)
        {
            url.clear();
        }
        if(url.find("localhost:") != std::string::npos)
        {
            url.clear();
        }
        if(url.find("..") != std::string::npos)
        {
            url.clear();
        }
        if(url.rfind(".7z") == url.length()-3)
        {
            url.clear();
        }
        if(url.rfind(".json") == url.length()-5)
        {
            url.clear();
        }
        if(url.rfind(".bz2") == url.length()-4)
        {
            url.clear();
        }
        if(url.rfind(".gz") == url.length()-3)
        {
            url.clear();
        }
        if(url.rfind(".zip") == url.length()-4)
        {
            url.clear();
        }
        if(url.rfind(".tar") == url.length()-3)
        {
            url.clear();
        }
        if(url.length()>300)
        {
            url.clear();
        }

        // Some URLs do not start with "http" or "https",
        // hence they are modified to include it.
        // TODO: how do we know whes is "http" or "https"?
        // This routine implements only one case.
        if(url.substr(0,2) == "//")
        {
            // url = url_prefix + url.substr(1);
            url = "https:" + url;
        }
        else if(url[0] == '/')
        {
            url = url_prefix + url;
        }

        // Finally, it must start with "http",
        // this includes also "https",
        // otherswise discard it.
        if(url.substr(0,4) != "http")
        {
            url.clear();
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << __PRETTY_FUNCTION__;
        std::cerr << "error:" << e.what() << '\n';
    }

    return url;
}

std::vector<std::string> Trokam::PlainTextProcessor::tokenize(
    std::string text,
    const char &delimiter)
{
    std::vector<std::string> result;
    // const char delimiter = ' ';
    size_t ini = 0;
    size_t pos = 0;

    text += delimiter;
    while((pos = text.find(delimiter, pos+1)) != std::string::npos)
    {
        std::string part = text.substr(ini, pos-ini);
        boost::algorithm::trim_if(
            part, boost::algorithm::is_any_of(" \n\r\t\\\""));
        if(!part.empty())
        {
            result.push_back(part);
        }
        ini = pos+1;
    }
    return result;
}

/**
std::string Trokam::PlainTextProcessor::snippet(
    const std::string &block_text,
    const std::string &search_text,
    const size_t &snippet_length)
{
    const std::vector<std::string> search_tokens =
        Trokam::PlainTextProcessor::tokenize(search_text);

    if(search_tokens.size() > 0)
    {
        size_t loc = caseInsensitiveFind(block_text, search_tokens[0]);
        if(loc == std::string::npos)
        {
            std::cout << "<not found>\n";
            if(snippet_length < block_text.length())
            {
                std::string result = block_text.substr(0, snippet_length);
                return result;
            }
            else
            {
                return block_text;
            }
        }
        else
        {
            int begin = loc - snippet_length/2;
            if(begin < 0)
            {
                begin = 0;
            }

            int end = loc + snippet_length/2;
            if(size_t(end) > block_text.length())
            {
                end = block_text.length();
            }

            std::string result = block_text.substr(begin, end-begin);
            return result;
        }
    }
    else
    {
        std::string result = block_text.substr(0, snippet_length);
        return result;
    }
}
**/

/**
 * @brief It returns a contiguous part of block_text of snippet_length
 *        lentgth where at least the first term of search_text appears.
 *
 * @param block_text Usually a large piece of text.
 * @param search_text The terms to search within block_text.
 * @param snippet_length The lenght of the snippet returned.
 * @return std::string A part of the block_text.
 */
std::string Trokam::PlainTextProcessor::snippet(
    const std::string &block_text,
    const std::string &search_text,
    const size_t &snippet_length)
{
    std::string result;

    // The search_text is split in individuals words.
    // The words are kept in a vector.
    const std::vector<std::string> search_tokens =
        Trokam::PlainTextProcessor::tokenize(search_text);

    if(search_tokens.size() > 0)
    {
        // Find the words in block_text.
        // loc is the location of the first occurrence.
        size_t loc = caseInsensitiveFind(block_text, search_tokens[0]);

        if(loc == std::string::npos)
        {
            // No one of the words was found.
            // The snippet is a portion of block_text
            // or block_text entirely.

            result = block_text.substr(0, snippet_length);

            /*
            if(snippet_length < block_text.length())
            {
                result = block_text.substr(0, snippet_length);
            }
            else
            {
                result = block_text;
            }
            */
        }
        else
        {
            int begin = loc - snippet_length/2;
            if(begin < 0)
            {
                begin = 0;
            }

            int end = loc + snippet_length/2;
            if(size_t(end) > block_text.length())
            {
                end = block_text.length();
            }

            result = block_text.substr(begin, end-begin);
        }
    }
    else
    {
        result = block_text.substr(0, snippet_length);
    }

    boost::replace_all(result, "   ", " ");
    boost::replace_all(result, "  ", " ");

    return result;
}

size_t Trokam::PlainTextProcessor::caseInsensitiveFind(
    const std::string &text_block,
    std::string text_piece)
{
    std::transform(
        text_piece.begin(),
        text_piece.end(),
        text_piece.begin(),
        ::toupper);

    std::string::const_iterator it =
        search(
            text_block.begin(),
            text_block.end(),
            text_piece.begin(),
            text_piece.end(),
            [](char a, char b){return toupper(a) == b;});

    if(it == text_block.end())
    {
        return std::string::npos;
    }
    return it - text_block.begin();
}

float Trokam::PlainTextProcessor::howMuchOf(
    std::string text_block,
    std::string text_piece)
{
    boost::algorithm::trim_if(
        text_block, boost::algorithm::is_any_of(" "));

    float original_length = text_block.length();
    std::vector<std::string> tokens = tokenize(text_piece);
    for(const auto &element: tokens)
    {
        size_t pos = caseInsensitiveFind(text_block, element);

        /**
        while(pos != std::string::npos)
        {
            text_block.erase(pos, element.length());
            pos = caseInsensitiveFind(text_block, element);
        }
        **/

        if(pos != std::string::npos)
        {
            text_block.erase(pos, element.length());
        }
    }
    boost::algorithm::trim_if(
        text_block, boost::algorithm::is_any_of(" "));
    float remaining_length = text_block.length();

    return 100.0 * (original_length-remaining_length)/original_length;
}
