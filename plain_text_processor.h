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

// C++
#include <iostream>
#include <string>
#include <vector>

// Boost
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>

// Trokam
#include "common.h"

class PlainTextProcessor
{
    public:

        static std::string get_url_prefix(
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

        static void extract_url(
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

        static std::string format_url(
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

       static std::string getTitle(
            const std::string &raw);
};
