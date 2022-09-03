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

    // input->textInput().connect(this, &Trokam::SearchWidget::textInput);
    input->textInput().connect(
        [=] {
            if(input && suggestions)
            {
                // std::string prefix= input->text().toUTF8();

                std::string full_text= input->text().toUTF8();
                std::vector<std::string> tokens =
                    Trokam::PlainTextProcessor::tokenize(full_text);

                if(tokens.size()>0)
                {
                    size_t last = tokens.size()-1;
                    std::string prefix= tokens[last];

                    std::string prev_tokens;
                    for(size_t i=0; i<last; i++)
                    {
                        prev_tokens += tokens[i] + " ";
                    }

                    Wt::log("info") << "prefix:" << prefix;

                    suggestions->clearSuggestions();

                    if(prefix.length() > 2)
                    {
                        auto data =
                            shared_resources->
                                readable_content_db.lookUp(prefix);

                        unsigned int max_results = 12; // opt.maxResults();

                        for(size_t i= 0; ((i<data.size()) && (i<max_results)); i++)
                        {
                            Wt::log("info") << "adding:" << std::get<0>(data[i]);
                            // suggestions->addSuggestion(std::get<0>(data[i]));
                            std::string full_line = prev_tokens + std::get<0>(data[i]);
                            suggestions->addSuggestion(full_line);
                        }
                    }
                }

                /**
                std::vector<Wt::WMenuItem*> items = phrasesPopup->items();
                for(Wt::WMenuItem *e: items)
                {
                    phrasesPopup->removeItem(e);
                }

                std::string prefix= input->text().toUTF8();
                Wt::log("info") << "prefix:" << prefix;

                phrasesPopup->setHidden(false);

                if(prefix.length() > 2)
                {
                    auto data =
                        shared_resources->
                            readable_content_db.lookUp(prefix);

                    unsigned int max_results = 12; // opt.maxResults();

                    for(size_t i= 0; ((i<data.size()) && (i<max_results)); i++)
                    {
                        Wt::log("info") << "adding:" << std::get<0>(data[i]);
                        phrasesPopup->addItem(std::get<0>(data[i]));
                    }

                    std::vector<Wt::WMenuItem*> alternatives= phrasesPopup->items();
                    for(unsigned int i=0; i<alternatives.size(); i++)
                    {
                        alternatives[i]->setCanReceiveFocus(true);
                        alternatives[i]->
                            keyWentDown().connect(
                                this,
                                &SearchPage::phrasesPopupKeyPressed);
                    }

                    phrasesPopup->popup(input);
                    phrasesPopup->setHidden(false);

                    // phrasesPopup->items()[0]->setFocus(true);
                }
                **/
            }
        });

    input->keyWentDown().connect(
        this, &Trokam::SearchPage::keyPressedInput);

    Wt::WSuggestionPopup::Options contactOptions;
    /**
    contactOptions.highlightBeginTag = "<span class=\"highlight\">";
    contactOptions.highlightEndTag = "</span>";
    **/

    contactOptions.highlightBeginTag = "<b>";
    contactOptions.highlightEndTag = "</b>";

    /*
    contactOptions.listSeparator = ',';
    contactOptions.whitespace = " \n";
    contactOptions.wordSeparators = "-., \"@\n;";
    */
    // contactOptions.appendReplacedText = ", ";

    suggestions =
        container->addChild(
        std::make_unique<Wt::WSuggestionPopup>(
                Wt::WSuggestionPopup::generateMatcherJS(contactOptions),
                Wt::WSuggestionPopup::generateReplacerJS(contactOptions)));
    // suggestions->setThemeStyleEnabled(false);
    suggestions->setAutoSelectEnabled(false);
    suggestions->setCanReceiveFocus(true);
    suggestions->activated().connect(
        this, &Trokam::SearchPage::suggestionSelected);

    /**
    suggestions->addStyleClass("Wt-popup");
    suggestions->addStyleClass("dropdown-menu");
    suggestions->addStyleClass("typeahead");
    suggestions->addStyleClass("wt-reparented");
    **/

    /*
    suggestions =
        container->addChild(
            std::make_unique<Wt::WSuggestionPopup>());
    */

    suggestions->forEdit(input);

    /**
    phrasesPopup = std::make_unique<Wt::WPopupMenu>();
    phrasesPopup->
        itemSelected().connect(
            this,
            &Trokam::SearchPage::phrasesPopupSelect);

    phrasesPopup->setCanReceiveFocus(true);
    phrasesPopup->setHidden(true);
    **/

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

    /*
    container->addWidget(
        std::make_unique<Wt::WTemplate>(
            Wt::WString::tr("search-results-example")));
    */

    userFindings =
        container->addWidget(std::make_unique<Wt::WTable>());

    /**
    container->addWidget(
        std::make_unique<Wt::WTemplate>(
            Wt::WString::tr("search-page-footer")));
    **/

    container->addWidget(
        std::make_unique<Wt::WTemplate>(
            Wt::WString::tr("search-page-footer")));

    createFooter(container);

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

    /**
    container->addNew<Wt::WAnchor>(
        Wt::WLink(Wt::LinkType::InternalPath, "/navigation/eat"), "Eat");
    **/

    /**
    auto trigger_search =
        container->
            addWidget(std::make_unique<Wt::WPushButton>("Go"));
    trigger_search->clicked().connect(
        [=] {
            application->setInternalPath("/search/cpp", true);
        });
    **/

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
    // search("cppcon");
    // show_search_results();
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

void Trokam::SearchPage::handlePathChange()
{
    Wt::log("info") << "current path:" << application->internalPath();
}

void Trokam::SearchPage::createFooter(
    Wt::WContainerWidget *base)
{
    int total_pages = 2;

    auto container_l0 =
        base->addWidget(std::make_unique<Wt::WContainerWidget>());
    container_l0->addStyleClass("mt-auto");
    container_l0->addStyleClass("text-center");

    auto container_l1 =
        container_l0->addWidget(std::make_unique<Wt::WContainerWidget>());
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

void Trokam::SearchPage::suggestionSelected(
    const int index,
    Wt::WFormWidget *widget)
{
    Wt::log("info") << __PRETTY_FUNCTION__;
    Wt::log("info") << "index=" << index;

    // std::shared_ptr<Wt::WAbstractItemModel> abstract_model= suggestions->model();
    auto abstract_model= suggestions->model();
    auto string_model= std::dynamic_pointer_cast<Wt::WStringListModel>(abstract_model);

    // D& new_d = dynamic_cast<D&>(a)
    // Wt::WStringListModel *model= dynamic_cast<Wt::WStringListModel*>(suggestions->model());
    // Wt::log("info") << "selected=" << string_model->stringList()[index];

    Wt::WString search_terms = string_model->stringList()[index];

    Wt::log("info") << "search_terms=" << search_terms.toUTF8();

    search(search_terms.toUTF8());
    show_search_results();
}

void Trokam::SearchPage::keyPressedInput(const Wt::WKeyEvent &kEvent)
{
    Wt::log("info") << __PRETTY_FUNCTION__;

    if ((kEvent.key() == Wt::Key::Down) && (suggestions))
    {
        Wt::log("info") << "A";

        // std::vector<Wt::WMenuItem*> alternatives= suggestions->items();
        // Wt::log("info") << "alternatives.size()=" << alternatives.size();

        // if(alternatives.size() > 0)
        {
            Wt::log("info") << "B";

            input->setFocus(false);
            suggestions->setFocus(true);
            // alternatives[0]->setFocus(true);
            phraseOnFocus= 0;

            // application->processEvents();
        }
    }

    /*
    if ((kEvent.key() == Wt::Key::Down) && (phrasesPopup))
    {
        Wt::log("info") << "A";

        std::vector<Wt::WMenuItem*> alternatives= phrasesPopup->items();

        Wt::log("info") << "alternatives.size()=" << alternatives.size();

        if(alternatives.size() > 0)
        {
            Wt::log("info") << "B";

            input->setFocus(false);
            phrasesPopup->setFocus(true);
            alternatives[0]->setFocus(true);
            phraseOnFocus= 0;

            application->processEvents();
        }
    }
    */
}

void Trokam::SearchPage::phrasesPopupKeyPressed(
    const Wt::WKeyEvent &kEvent)
{
    Wt::log("info") << __PRETTY_FUNCTION__;

    /*
    if(kEvent.key() == Wt::Key::Down)
    {
        phraseOnFocus++;
        if((0 <= phraseOnFocus) && (phraseOnFocus < phrasesPopup->count()))
        {
            std::vector<Wt::WMenuItem*> alternatives= phrasesPopup->items();
            alternatives[phraseOnFocus]->setFocus(true);
        }
        else
        {
            phraseOnFocus--;
        }
    }
    else if(kEvent.key() == Wt::Key::Up)
    {
        phraseOnFocus--;
        if((0 <= phraseOnFocus) && (phraseOnFocus < phrasesPopup->count()))
        {
            std::vector<Wt::WMenuItem*> alternatives= phrasesPopup->items();
            alternatives[phraseOnFocus]->setFocus(true);
        }
        else if (phraseOnFocus == -1)
        {
            input->setFocus(true);
        }
    }
    else if(kEvent.key() == Wt::Key::Enter)
    {
        if(phrasesPopup)
        {
            phrasesPopup->select(phraseOnFocus);
        }
    }
    else
    {
        Wt::log("info") << "other key";
    }
    */
}

void Trokam::SearchPage::phrasesPopupSelect(Wt::WMenuItem *item)
{
    Wt::log("info") << __PRETTY_FUNCTION__;

    /*
    phraseOnFocus= -1;

    const std::string choice= item->text().toUTF8();
    input->setText(choice);
    input->setFocus(true);

    application->processEvents();

    search(choice);
    show_search_results();
    // generateFooter();
    */
}
