/***********************************************************************
 *                            T R O K A M
 *                       Internet Search Engine
 *
 * Copyright (C) 2017, Nicolas Slusarenko
 *                     nicolas.slusarenko@trokam.com
 *
 * Copyright (C) 2017, Emweb bvba, Heverlee, Belgium.
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

// Wt
#include <Wt/WApplication.h>
#include <Wt/WEnvironment.h>
#include <Wt/WPushButton.h>
#include <Wt/WContainerWidget.h>
// #include <Wt/WHBoxLayout.h>
// #include <Wt/WImage.h>
// #include <Wt/WMenu.h>
// #include <Wt/WNavigationBar.h>
// #include <Wt/WLineEdit.h>
// #include <Wt/WStackedWidget.h>
#include <Wt/WString.h>
#include <Wt/WText.h>
// #include <Wt/WVBoxLayout.h>
#include <Wt/WTemplate.h>

// Trokam
#include "aboutWidget.h"
#include "searchWidget.h"
#include "ackWidget.h"
#include "donWidget.h"
#include "searchPage.h"
#include "sharedResources.h"
#include "preferences.h"

Trokam::SearchPage::SearchPage(
    boost::shared_ptr<Trokam::SharedResources> &sr,
    Wt::WApplication* app):
        Wt::WContainerWidget(),
        application(app),
        shared_resources(sr)
{
    Wt::log("info") << "SearchPage constructor";

    addStyleClass("d-flex");
    addStyleClass("h-100");
    // addStyleClass("text-bg-dark");
    addStyleClass("text-bg-primary");

    auto container = addWidget(std::make_unique<Wt::WContainerWidget>());

    container->addStyleClass("cover-container");
    container->addStyleClass("d-flex");
    container->addStyleClass("w-100");
    container->addStyleClass("h-100");
    container->addStyleClass("p-3");
    container->addStyleClass("mx-auto");
    container->addStyleClass("flex-column");

    container->addWidget(
        std::make_unique<Wt::WTemplate>(
            Wt::WString::tr("search-page-header")));

    /*
    container->addWidget(
        std::make_unique<Wt::WTemplate>(
            Wt::WString::tr("search-results-example")));
    */

    userFindings =
        container->addWidget(std::make_unique<Wt::WTable>());

    container->addWidget(
        std::make_unique<Wt::WTemplate>(
            Wt::WString::tr("search-page-footer")));

    /***
     * Implement this using paths
    footer =
        container->addWidget(std::make_unique<Wt::WContainerWidget>());
    footer->addStyleClass("mt-auto ")
    footer->addStyleClass("text-center")
    ***/

    /*
    container->addWidget(
        std::make_unique<Wt::WTemplate>(
            Wt::WString::tr("search-full-page")));
    */

    auto change_style =
        container->
            addWidget(std::make_unique<Wt::WPushButton>("Change style"));
    change_style->clicked().connect(change_style, &Wt::WPushButton::disable);
    change_style->clicked().connect(
        [=] {
            const Wt::WEnvironment& env = application->environment();
            std::string page_style = "light";
            if(env.getCookie("page_style") != nullptr)
            {
                page_style= *(env.getCookie("page_style"));
            }

            if(page_style == "light")
            {
                application->setCookie("page_style", "dark", 10000000);
            }
            else
            {
                application->setCookie("page_style", "light", 10000000);
            }
            // application->refresh();
        });
    search("cppcon");
    show_search_results();
}

void Trokam::SearchPage::search(
    const std::string &terms)
{
    Wt::log("info") << "+++++++ search() -- terms:" << terms;

    std::string low_case_terms= Xapian::Unicode::tolower(terms);

    Xapian::doccount results_requested = 24;

    /*
    std::vector<std::string> language_selected;
    for(unsigned int i=0; i<language_options.size(); i++)
    {
        if(std::get<bool>(language_options[i]))
        {
            const std::string language_name =
                Trokam::Preferences::languageName(i);
            language_selected.push_back(language_name);
        }
    }
    */

    std::vector<std::string> language_selected;
    language_selected.push_back("english");

    items_found =
        shared_resources->
            readable_content_db.search(
                low_case_terms,
                language_selected,
                results_requested);

    Wt::log("info") << "+++++++ total results:" << items_found.size();

    // current_page = 1;
}

void Trokam::SearchPage::show_search_results()
{
    Wt::log("info") << "+++++++ show_search_results()";

    int total_results = items_found.size();
    auto dv = std::div(total_results, results_per_page);
    int total_pages = dv.quot;
    if(dv.rem > 0)
    {
        total_pages++;
    }

    Wt::log("info") << "dv.quot:" << dv.quot;
    Wt::log("info") << "dv.rem:" << dv.rem;

    userFindings->clear();
    if(items_found.size() != 0)
    {
        unsigned int ini = (current_page-1) * results_per_page;
        unsigned int end = ini + results_per_page;
        if((dv.rem > 0) && (current_page == total_pages))
        {
            end = ini + dv.rem;
        }

        for(unsigned int i=ini; i<end; i++)
        {
            std::string out;
            // out+= "<p>&nbsp;<br/>";
            /**
            out+= "<a href=\"" + items_found[i].url + "\" target=\"_blank\"><span style=\"font-size:x-large;\">" + items_found[i].title + "</span></a><br/>";
            out+= "<strong><a href=\"" + items_found[i].url + "\" target=\"_blank\">" + items_found[i].url + "</a></strong><br/>";
            out+= items_found[i].snippet + "<br/>";
            out+= "<span class=\"text-success\">";
            out+= "</strong>[" + std::to_string((int)i) + "]<strong>";
            out+= "</strong> relevance body:<strong>" + std::to_string((int)items_found[i].relevance_body);
            out+= "</strong> relevance URL:<strong>" + std::to_string((int)items_found[i].relevance_url);
            out+= "</strong> relevance title:<strong>" + std::to_string((int)items_found[i].relevance_title);
            out+= "</strong> total:<strong>" + std::to_string((int)items_found[i].relevance_total);
            out+= "</strong></span><br/>";
            out+= "</p>";
            **/
            // out+= "&nbsp;<br/>";

            out+= "<h2 class=\"page-title\"><a href=\"" + items_found[i].url + "\" target=\"_blank\">" + items_found[i].title + "</a></h2>";
            out+= "<a href=\"" + items_found[i].url + "\" target=\"_blank\">" + items_found[i].url + "</a><br/>";
            out+= "<span class=\"snippet\">" + items_found[i].snippet + "</strong></span><br/>";
            out+= "<span class=\"ranking\">";
            out+= "</strong>[" + std::to_string((int)i) + "]<strong>";
            out+= "</strong> relevance body:<strong>" + std::to_string((int)items_found[i].relevance_body);
            out+= "</strong> relevance URL:<strong>" + std::to_string((int)items_found[i].relevance_url);
            out+= "</strong> relevance title:<strong>" + std::to_string((int)items_found[i].relevance_title);
            out+= "</strong> total:<strong>" + std::to_string((int)items_found[i].relevance_total);
            out+= "</strong></span><br/>";
            out+= "</p>";

            // auto oneRow = std::make_unique<Wt::WTemplate>(out);
            auto oneRow = std::make_unique<Wt::WTemplate>();
            oneRow->setTemplateText(out, Wt::TextFormat::UnsafeXHTML);
            userFindings->elementAt(i, 0)->addWidget(std::move(oneRow));
        }
    }
    else
    {
        // Tell that there are not any results found.
    }
}
