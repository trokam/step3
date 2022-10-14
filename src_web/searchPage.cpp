/***********************************************************************
 *                            T R O K A M
 *                       Internet Search Engine
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

// Wt
#include <Wt/WAbstractItemModel.h>
#include <Wt/WAnchor.h>
#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WEnvironment.h>
#include <Wt/WPopupMenu.h>
#include <Wt/WPushButton.h>
// #include <Wt/WHBoxLayout.h>
// #include <Wt/WImage.h>
// #include <Wt/WMenu.h>
// #include <Wt/WNavigationBar.h>
// #include <Wt/WLineEdit.h>
// #include <Wt/WStackedWidget.h>
#include <Wt/WLink.h>
#include <Wt/WString.h>
// #include <Wt/WVBoxLayout.h>
#include <Wt/WStringListModel.h>
#include <Wt/WTemplate.h>
#include <Wt/WText.h>
#include <Wt/Utils.h>

// Xapian
#include <xapian.h>

// Trokam
#include "aboutWidget.h"
#include "searchWidget.h"
#include "ackWidget.h"
#include "donWidget.h"
#include "preferences.h"
#include "plain_text_processor.h"
#include "searchPage.h"
#include "sharedResources.h"

#define INLINE_JAVASCRIPT(...) #__VA_ARGS__

Trokam::SearchPage::SearchPage(
    boost::shared_ptr<Trokam::SharedResources> &sr,
    Wt::WApplication* app):
        Wt::WContainerWidget(),
        application(app),
        shared_resources(sr)
{
    Wt::log("info") << "SearchPage constructor";

    application->internalPathChanged().connect(
        [=] {
            // handlePathChange();
            Wt::log("info") << "current path:" << application->internalPath();
            // const std::string internalPath= application->internalPath();
            // std::string::size_type loc= internalPath.find("search?terms=");
            // if(loc != std::string::npos)

            std::string internal_path= application->internalPath();
            boost::algorithm::trim_if(
                internal_path, boost::algorithm::is_any_of("/ \n\r\t\\\""));

            // const std::string search_terms = internalPath.substr(loc + 13);
            // const std::string search_terms=
            //     Trokam::PlainTextProcessor::urlDecode(internal_path);

            const std::string search_terms=
                Wt::Utils::urlDecode(internal_path);

            Wt::log("info") << "decoded search_terms:" << search_terms;
            search(search_terms);
            show_search_results();
        });

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

    auto header = container->addWidget(
        std::make_unique<Wt::WTemplate>(
            Wt::WString::tr("search-page-header")));

    input = header->bindWidget(
        "input",
        std::make_unique<Wt::WLineEdit>());
    // input = container->addWidget(std::make_unique<Wt::WLineEdit>());
    // input->addStyleClass("form-control");
    input->setPlaceholderText("Search for ...");
    input->enterPressed().connect(
        [=] {
            std::string user_input= input->text().toUTF8();
            Wt::log("info") << "user_input:" << user_input;
            user_input = Xapian::Unicode::tolower(user_input);
            Wt::log("info") << "user_input:" << user_input;
            const std::string encoded_terms= Wt::Utils::urlEncode(user_input);
            Wt::log("info") << "encoded terms:" << encoded_terms;
            std::string internal_url = "/";
            internal_url+= encoded_terms;
            application->setInternalPath(internal_url, true);
        });

    auto button = header->bindWidget(
        "button",
        std::make_unique<Wt::WPushButton>("search"));
    button->addStyleClass("btn");
    button->addStyleClass("btn-outline-secondary");
    button->clicked().connect(
        [=] {

            // cpp.cgi?key1=value1&key2=value2

            std::string user_input= input->text().toUTF8();
            Wt::log("info") << "user_input:" << user_input;
            user_input = Xapian::Unicode::tolower(user_input);
            Wt::log("info") << "user_input:" << user_input;
            const std::string encoded_terms= Wt::Utils::urlEncode(user_input);
            Wt::log("info") << "encoded terms:" << encoded_terms;
            std::string internal_url = "/";
            internal_url+= encoded_terms;
            application->setInternalPath(internal_url, true);
        });

    userFindings =
        container->addWidget(std::make_unique<Wt::WTable>());

    container->addWidget(
        std::make_unique<Wt::WTemplate>(
            Wt::WString::tr("search-page-footer")));

    serverSideFilteringPopups(container);
    createFooter(container);

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

    Wt::log("info") << "current path:" << application->internalPath();
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

    shared_resources->getNewDB();

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

void Trokam::SearchPage::handlePathChange()
{
    Wt::log("info") << "current path:" << application->internalPath();
}

void Trokam::SearchPage::createFooter(
    Wt::WContainerWidget *base)
{
    int total_pages = 2;

    if(w_footer != nullptr)
    {
        delete w_footer;
    }

    auto w_footer =
        base->addWidget(std::make_unique<Wt::WContainerWidget>());
    w_footer->addStyleClass("mt-auto");
    w_footer->addStyleClass("text-center");

    auto container_l1 =
        w_footer->addWidget(std::make_unique<Wt::WContainerWidget>());
    container_l1->addStyleClass("lead");

    auto button_previous =
        container_l1->addWidget(std::make_unique<Wt::WPushButton>());
    button_previous->setStyleClass("paging-button");
    button_previous->setTextFormat(Wt::TextFormat::XHTML);
    button_previous->setText("<span class=\"paging-text\">Previous</span>");
    button_previous->clicked().connect(
        [=] {
            Wt::log("info") << "previous";
            if(current_page > 1)
            {
                current_page--;
                show_search_results();
                createFooter(base);
            }
        });

    container_l1->addWidget(std::make_unique<Wt::WText>("&nbsp;"));

    auto button_1 =
        container_l1->addWidget(std::make_unique<Wt::WPushButton>());
    button_1->setStyleClass("paging-button");
    button_1->setTextFormat(Wt::TextFormat::XHTML);
    button_1->setText("<span class=\"paging-text\">1</span>");
    button_1->clicked().connect(
        [=] {
            current_page = 1;
            show_search_results();
            createFooter(base);
        });

    container_l1->addWidget(std::make_unique<Wt::WText>("&nbsp;"));

    auto button_2 =
        container_l1->addWidget(std::make_unique<Wt::WPushButton>());
    button_2->setStyleClass("paging-button");
    button_2->setTextFormat(Wt::TextFormat::XHTML);
    button_2->setText("<span class=\"paging-text\">2</span>");
    button_2->clicked().connect(
        [=] {
            current_page = 2;
            show_search_results();
            createFooter(base);
        });

    container_l1->addWidget(std::make_unique<Wt::WText>("&nbsp;"));

    auto button_next =
        container_l1->addWidget(std::make_unique<Wt::WPushButton>());
    button_next->addStyleClass("paging-button");
    button_next->setTextFormat(Wt::TextFormat::XHTML);
    button_next->setText("<span class=\"paging-text\">Next</span>");
    button_next->clicked().connect(
        [=] {
            Wt::log("info") << "next";
            if(current_page < total_pages)
            {
                current_page++;
                show_search_results();
                createFooter(base);
            }
        });
}

void Trokam::SearchPage::serverSideFilteringPopups(
    WContainerWidget *parent)
{
    fourCharModel_ = std::make_shared<Wt::WStringListModel>();

    Wt::WSuggestionPopup *popup = createAliasesMatchingPopup(parent);
    popup->setModel(fourCharModel_);
    popup->setFilterLength(3);
    popup->filterModel().connect(this, &SearchPage::filter);
    popup->setMinimumSize(150, Wt::WLength::Auto);
    popup->setMaximumSize(Wt::WLength::Auto, 300);

    parent->addWidget(std::make_unique<Wt::WText>(Wt::WString::tr("serverside-popup-editing")));

    popup->forEdit(input, Wt::PopupTrigger::Editing);
}

void Trokam::SearchPage::filter(const Wt::WString& input)
{
    /*
        * We implement a virtual model contains all items that start with
        * any arbitrary 3 characters, followed by "a-z"
        */
    fourCharModel_->removeRows(0, fourCharModel_->rowCount());

    for (int i = 0; i < 26; ++i)
    {
        int row = fourCharModel_->rowCount();

        /*
            * If the input is shorter than the server-side filter length,
            * then limit the number of matches and end with a '...'
            */
        if (input.value().length() < 3 && i > 10)
        {
            fourCharModel_->addString("...");
            fourCharModel_->setData(row, 0, std::string(""),              Wt::ItemDataRole::User);
            fourCharModel_->setData(row, 0, std::string("Wt-more-data"),  Wt::ItemDataRole::StyleClass);
            break;
        }

        std::u32string v = input;
        while (v.length() < 3)
            v += U"a";

        v += (U"a"[0] + i);

        fourCharModel_->addString(v);
    }
}

Wt::WSuggestionPopup* Trokam::SearchPage::createAliasesMatchingPopup(
    WContainerWidget *parent)
{
/*
    * This matcher JavaScript function matches the input against the
    * name of a product, or one or more aliases.
    *
    * A match is indicated by product name and optionally matching aliases
    * between brackets.
    */

/*
    * Note!
    *
    * INLINE_JAVASCRIPT is a macro which allows entry of JavaScript
    * directly in a C++ file.
    */
std::string matcherJS = INLINE_JAVASCRIPT
    (
    function (edit) {
        var value = edit.value;

        return function(suggestion) {
        if (!suggestion)
            return value;

        var i, il,
            names = suggestion.split(';'),
            val = value.toUpperCase(),
            matchedAliases = [],
            matched = null;

        if (val.length) {
            for (i = 0, il = names.length; i < il; ++i) {
            var name = names[i];
            if (name.length >= val.length
                && name.toUpperCase().substr(0, val.length) == val) {
                // This name matches
                name = '<b>' + name.substr(0, val.length) + '</b>'
                + name.substr(val.length);

                if (i == 0) // it's the product name
                matched = name;
                else // it's an alias
                matchedAliases.push(name);
            }
            }
        }

        // Let '...' always match
        if (names[0] == '...')
            matched = names[0];

        if (matched || matchedAliases.length) {
            if (!matched)
            matched = names[0];

            if (matchedAliases.length)
            matched += " (" + matchedAliases.join(", ") + ")";

            return { match : true,
                    suggestion : matched };
        } else {
            return { match : false,
                    suggestion : names[0] };
        }
        }
    }
    );

std::string replacerJS = INLINE_JAVASCRIPT
    (
    function (edit, suggestionText, suggestionValue) {
        edit.value = suggestionValue;

        if (edit.selectionStart)
        edit.selectionStart = edit.selectionEnd = suggestionValue.length;
    }
    );

    return parent->addChild(
        std::make_unique<Wt::WSuggestionPopup>(matcherJS, replacerJS));
}
