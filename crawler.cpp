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
#include <string>
#include <vector>

// Trokam
#include "crawler.h"
#include "doc_processor.h"

/**
 * Called by libcurl as soon as there is data received that
 * needs to be saved. More info:
 * https://curl.se/libcurl/c/CURLOPT_WRITEFUNCTION.html
 */
static int append_data(
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

void Trokam::Crawler::run()
{
    // The database that keeps the URLs and their state: pending, downloaded, etc.
    Warehouse house;

    // Gets a bundle of URLs for downloading.
    // Try to get TOTAL_PER_RUN number of URLs, but it may get less than that.
    // 'bundle_size' is the the actual number of URLs in the 'url_bundle'.
    std::vector<std::tuple<std::string, int>> url_bundle = house.get_bundle(TOTAL_PER_RUN);
    size_t bundle_size = url_bundle.size();

    // Initialise the curl library for parallel downloading.
    CURLM *curl_multi_handler;
    curl_global_init(CURL_GLOBAL_ALL);
    curl_multi_handler = curl_multi_init();

    // Limit the amount of simultaneous connections libcurl should allow.
    curl_multi_setopt(curl_multi_handler, CURLMOPT_MAXCONNECTS, MAX_PARALLEL);

    // The number of URLs set for processing in this run.
    size_t set_for_download = 0;

    // Set URLs to start the downloading.
    for(size_t i=0; i<MAX_PARALLEL; i++)
    {
        if(i<url_bundle.size())
        {
            std::tuple<std::string, int> result = url_bundle[i];
            setup_download(curl_multi_handler, std::get<0>(result), std::get<1>(result));

            // Increment URLs set for processing in this run.
            set_for_download++;
        }
    }

    // This is the processor of the downloaded data.
    Trokam::DocProcessor processor;

    CURLMsg *transfer_info;
    int msgs_left = -1;
    int still_working = 1;

    // The number of actual downloads
    size_t downloaded = 0;

    do
    {
        // Perform transfers on all the added handles that need attention
        // in an non-blocking fashion. It will store the number of handles
        // that still transfer data in the second argument's integer-pointer.
        // More info: https://curl.se/libcurl/c/curl_multi_perform.html
        // TODO: Use the returning value. When this function returns error, the state
        // of all transfers are uncertain and they cannot be continued.
        curl_multi_perform(curl_multi_handler, &still_working);

        // Ask the multi handle if there are any messages or informationals
        // from the individual transfers. Repeated calls to this function will
        // return a new struct each time, until a NULL is returned as a signal
        // that there is no more to get at this point. The integer pointed to 'msgs_left'
        // will contain the number of remaining messages after this function was called. 
        // More info: https://curl.se/libcurl/c/curl_multi_info_read.html
        while((transfer_info = curl_multi_info_read(curl_multi_handler, &msgs_left)))
        {
            if(transfer_info->msg == CURLMSG_DONE)
            {
                // Get the data of one individual transfer.
                // When this tranfer was added to the the multihandler, a pointer of
                // type web_doc was set. Now, we recover the same pointer with the
                // data retrieved, for instance a web page.
                web_doc *doc = nullptr;
                curl_easy_getinfo(transfer_info->easy_handle, CURLINFO_PRIVATE, &doc);

                // Get the error message of this individual transfer.
                const std::string retrieval_error = curl_easy_strerror(transfer_info->data.result);

                // Process the data of an individual transfer.
                processor.show(doc, retrieval_error, downloaded);

                // Remove the handle of this transfer.
                CURL *individual_handle = transfer_info->easy_handle;
                curl_multi_remove_handle(curl_multi_handler, individual_handle);
                curl_easy_cleanup(individual_handle);

                // Increment the number of URLs downloaded.
                downloaded++;
            }
            else
            {
                std::cerr << "fail: transfer information -- " <<  transfer_info->msg << '\n';
            }

            // Take an URL from the bundle and set it up for download.
            if(set_for_download < bundle_size)
            {
                std::tuple<std::string, int> result = url_bundle[set_for_download];
                setup_download(curl_multi_handler, std::get<0>(result), std::get<1>(result));

                std::cout << '\n';
                std::cout << "==== adding: " << std::get<1>(result) << '\n';
                std::cout << "==== adding: " << std::get<0>(result) << '\n';

                // Increment URLs set for processing in this run.
                set_for_download++;
            }
        }

        if(still_working)
        {
            curl_multi_wait(curl_multi_handler, NULL, 0, 1000, NULL);
        }
    }
    while(still_working || (set_for_download < bundle_size));

    curl_multi_cleanup(curl_multi_handler);
    curl_global_cleanup();
}

void Trokam::Crawler::setup_download(
    CURLM *curl_multi_handler,
    const std::string &url,
    const int &id)
{
    web_doc *doc = new web_doc;
    doc->url = url;
    doc->id = id;

    // Setting up new downloads.
    // More info about 'curl_easy_setopt':
    // https://curl.se/libcurl/c/curl_easy_setopt.html
    CURL *eh = curl_easy_init();
    curl_easy_setopt(eh, CURLOPT_WRITEFUNCTION, append_data);
    curl_easy_setopt(eh, CURLOPT_WRITEDATA, &(doc->raw));
    curl_easy_setopt(eh, CURLOPT_URL, url.c_str());
    curl_easy_setopt(eh, CURLOPT_TIMEOUT, 20L);
    curl_easy_setopt(eh, CURLOPT_PRIVATE, doc);
    curl_easy_setopt(eh, CURLOPT_USERAGENT, "Trokambot/1.0 (+http://trokam.com/bot.html)");
    curl_multi_add_handle(curl_multi_handler, eh);
}
