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

std::string Trokam::PlainTextProcessor::get_url_prefix(
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

void Trokam::PlainTextProcessor::extract_url(
    const int &max_url_extracted,
    const web_doc *doc,
    std::vector<std::string> &internal,
    std::vector<std::string> &external)
{
    boost::regex e(
        "<\\s*A\\s+[^>]*href\\s*=\\s*\"([^\"]*)\"",
        boost::regex::normal | boost::regbase::icase);

    int count= 0;
    std::string url_prefix = PlainTextProcessor::get_url_prefix(doc->url);
    boost::sregex_token_iterator i(doc->raw.begin(), doc->raw.end(), e, 1);
    boost::sregex_token_iterator j;
    while((i != j) && (count<max_url_extracted))
    {
        std::string url= *i;    
        url = format_url(url_prefix, url);

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

std::string Trokam::PlainTextProcessor::format_url(
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

std::string Trokam::PlainTextProcessor::getTitle(
    const std::string &raw)
{
    std::string title = "(empty)";
    try
    {
        std::string::size_type ini= raw.find("<title") + 6;
        if (ini != std::string::npos)
        {
            ini = raw.find(">", ini) + 1;
        }

        std::string::size_type end= raw.find("</title>", ini);
        if((ini != std::string::npos) && (end != std::string::npos))
        {
            if((end-ini) < 200)
            {
                title= raw.substr(ini, end-ini);
            }
            else
            {
                title= raw.substr(ini, 200);
            }
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << __PRETTY_FUNCTION__;
        std::cerr << "error:" << e.what() << '\n';
    }

    boost::algorithm::trim_if(title, boost::algorithm::is_any_of("\n\r\t\\\""));

    return title;
}

void Trokam::PlainTextProcessor::extractPlainText(
    const size_t &limit,
    const std::string &raw,
    std::string &text)
{
    text.reserve(20000);

    // If the page do not have a 'body', there is nothing to parse.
    std::string::size_type curr_loc = 0;
    curr_loc = raw.find("<body", curr_loc);
    if(curr_loc == std::string::npos)
    {
        return;
    }

    curr_loc += 4;

    try
    {
        while(
            (curr_loc < raw.size()) &&
            (curr_loc < 1000000) &&
            (text.length() < limit))
        {
            while(raw.at(curr_loc) != '>')
            {
                curr_loc++;
            }

            text+= ' ';

            // We know that the character is '>', them we move
            // one position forward.
            curr_loc++;

            while(raw.at(curr_loc) != '<')
            {
                text += raw.at(curr_loc);
                curr_loc++;
            }

            // We know that the current character is '<'.
            // We pay attention to the tag, because in
            // in some cases we discard its content.
            if(raw.substr(curr_loc, 6) == "<style")
            {
                std::string::size_type loc= raw.find("</style>", curr_loc);
                if(loc == std::string::npos)
                {
                    break;
                }
                curr_loc= loc + 8;
            }
            else if(raw.substr(curr_loc, 7) == "<script")
            {
                std::string::size_type loc= raw.find("</script>", curr_loc);
                if(loc == std::string::npos)
                {
                    break;
                }
                curr_loc= loc + 9;
            }
            else if(raw.substr(curr_loc, 6) == "</body")
            {
                break;
            }
            else if(raw.substr(curr_loc, 6) == "</html")
            {
                break;
            }
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << __PRETTY_FUNCTION__;
        std::cerr << "error:" << e.what() << '\n';
    }
}
