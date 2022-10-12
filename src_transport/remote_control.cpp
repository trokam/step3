/***********************************************************************
 *                            T R O K A M
 *                      trokam.com / trokam.org
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
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

// Trokam
#include "remote_control.h"

/**
 * Called by libcurl as soon as there is data received that
 * needs to be saved. More info:
 * https://curl.se/libcurl/c/CURLOPT_WRITEFUNCTION.html
 */
static int appendData(
    char *incoming_data,
    size_t size,
    size_t nmemb,
    std::string *current_data)
{
    if(current_data == NULL)
    {
        // TODO: report an error.
        return 0;
    }

    current_data->append(incoming_data, size*nmemb);
    return size * nmemb;
}

Trokam::RemoteControl::RemoteControl(const nlohmann::json &config)
{
    curl = curl_easy_init();

    std::string header_0 = "Content-Type: application/json";
    headers.push_back(header_0);

    const std::string auth_token = config["auth_token"];
    std::string header_1 = "Authorization: Bearer " + auth_token;
    headers.push_back(header_1);
}

Trokam::RemoteControl::~RemoteControl()
{
    if(curl != NULL)
    {
        curl_easy_cleanup(curl);
    }
}

std::string Trokam::RemoteControl::exec_post(
    const std::string &url,
    const std::string &data)
{
    std::string raw;
    struct curl_slist *list = NULL;

    for(size_t i=0; i<headers.size(); i++)
    {
        list = curl_slist_append(list, headers[i].c_str());
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data.length());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, appendData);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &raw);

    /* Perform the request, res will get the return code */
    CURLcode res = curl_easy_perform(curl);
    if(res != CURLE_OK)
    {
        BOOST_LOG_TRIVIAL(fatal) << "libcurl fail:" << curl_easy_strerror(res);
        exit(1);
    }

    if(list != NULL)
    {
        curl_slist_free_all(list);
    }

    return raw;
}

std::string Trokam::RemoteControl::exec_delete_vol(
    const std::string &url)
{
    std::string raw;
    struct curl_slist *list = NULL;

    for(size_t i=0; i<headers.size(); i++)
    {
        list = curl_slist_append(list, headers[i].c_str());
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, appendData);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &raw);

    /* Perform the request, res will get the return code */
    CURLcode res = curl_easy_perform(curl);
    if(res != CURLE_OK)
    {
        BOOST_LOG_TRIVIAL(fatal) << "libcurl fail:" << curl_easy_strerror(res);
        exit(1);
    }

    if(list != NULL)
    {
        curl_slist_free_all(list);
    }

    return raw;
}

std::string Trokam::RemoteControl::exec_get(
    const std::string &url)
{
    std::string raw;
    struct curl_slist *list = NULL;

    for(size_t i=0; i<headers.size(); i++)
    {
        list = curl_slist_append(list, headers[i].c_str());
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, appendData);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &raw);

    /* Perform the request, res will get the return code */
    CURLcode res = curl_easy_perform(curl);
    if(res != CURLE_OK)
    {
        BOOST_LOG_TRIVIAL(fatal) << "libcurl fail:" << curl_easy_strerror(res);
        exit(1);
    }

    if(list != NULL)
    {
        curl_slist_free_all(list);
    }

    return raw;
}
