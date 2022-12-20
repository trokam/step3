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
#include <chrono>

// Boost
#include <boost/algorithm/string.hpp>

// Wt
#include <Wt/WAbstractItemModel.h>
#include <Wt/WAnchor.h>
#include <Wt/WApplication.h>
#include <Wt/WButtonGroup.h>
#include <Wt/WCheckBox.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WEnvironment.h>
#include <Wt/WPopupMenu.h>
#include <Wt/WPushButton.h>
#include <Wt/WLabel.h>
#include <Wt/WLink.h>
#include <Wt/WLineEdit.h>
#include <Wt/WRadioButton.h>
#include <Wt/WString.h>
#include <Wt/WStringListModel.h>
#include <Wt/WTable.h>
#include <Wt/WTabWidget.h>
#include <Wt/WTemplate.h>
#include <Wt/WText.h>
#include <Wt/Utils.h>

// Xapian
#include <xapian.h>

// Trokam
#include "plain_text_processor.h"
#include "search_page.h"
#include "shared_resources.h"

Trokam::SearchPage::SearchPage(
    boost::shared_ptr<Trokam::SharedResources> &sr,
    Wt::WApplication* app):
        Wt::WContainerWidget(),
        application(app),
        shared_resources(sr)
{
    const Wt::WEnvironment& env = Wt::WApplication::instance()->environment();

    std::string cookie_preferences;
    if(env.getCookie("preferences"))
    {
        cookie_preferences= *(env.getCookie("preferences"));
    }
    else
    {
        Wt::log("info") << "cookie preferences = NULL";
    }

    user_settings.generate(cookie_preferences);
    application->internalPathChanged().connect(
        [=] {
            std::string internal_path= application->internalPath();
            boost::algorithm::trim_if(
                internal_path, boost::algorithm::is_any_of("/ \n\r\t\\\""));
            const std::string search_terms=
                Wt::Utils::urlDecode(internal_path);
            search(search_terms);
            show_search_results();
        });

    addStyleClass("d-flex");
    addStyleClass("h-100");
    addStyleClass("text-bg-primary");

    container = addWidget(std::make_unique<Wt::WContainerWidget>());
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
    input->setPlaceholderText("Search for ...");
    input->enterPressed().connect(
        [=] {
            timer->stop();
            destroySuggestionBox();
            std::string user_input= input->text().toUTF8();
            user_input = Xapian::Unicode::tolower(user_input);
            const std::string encoded_terms= Wt::Utils::urlEncode(user_input);
            std::string internal_url = "/";
            internal_url+= encoded_terms;
            application->setInternalPath(internal_url, true);
        });

    input->keyWentUp().connect(this, &Trokam::SearchPage::inputKeyWentUp);

    if(!isAgentMobile())
    {
        input->textInput().connect(this, &Trokam::SearchPage::textInput);
    }

    auto button = header->bindWidget(
        "button",
        std::make_unique<Wt::WPushButton>("search"));
    button->addStyleClass("btn");
    button->addStyleClass("btn-outline-secondary");
    button->clicked().connect(
        [=] {
            std::string user_input= input->text().toUTF8();
            user_input = Xapian::Unicode::tolower(user_input);
            const std::string encoded_terms= Wt::Utils::urlEncode(user_input);
            std::string internal_url = "/";
            internal_url+= encoded_terms;
            application->setInternalPath(internal_url, true);
        });

    w_button_preferences = header->bindWidget(
        "button_preferences",
        std::make_unique<Wt::WPushButton>());
    w_button_preferences->addStyleClass("paging-button");
    w_button_preferences->setTextFormat(Wt::TextFormat::XHTML);
    w_button_preferences->setText("<span class=\"paging-text\">Preferences</span>");
    w_button_preferences->
        clicked().connect(this, &Trokam::SearchPage::showUserOptions);

    w_button_sponsors = header->bindWidget(
        "button_sponsors",
        std::make_unique<Wt::WPushButton>());
    w_button_sponsors->addStyleClass("paging-button");
    w_button_sponsors->setTextFormat(Wt::TextFormat::XHTML);
    w_button_sponsors->setText("<span class=\"paging-text\">Sponsors</span>");
    w_button_sponsors->setLink(Wt::WLink(Wt::LinkType::Url, "/about.html"));

    w_about = header->bindWidget(
        "button_about",
        std::make_unique<Wt::WPushButton>());
    w_about->addStyleClass("paging-button");
    w_about->setTextFormat(Wt::TextFormat::XHTML);
    w_about->setText("<span class=\"paging-text\">About</span>");
    w_about->setLink(Wt::WLink(Wt::LinkType::Url, "/about.html"));

    userFindings =
        container->addWidget(std::make_unique<Wt::WTable>());

    createFooter(container);

    /**
     * Initialize searched languages.
     **/
    for(unsigned int i=0; i<LANGUAGES_TOTAL; i++)
    {
        auto language_item = std::make_pair(i, false);
        language_options.push_back(language_item);
    }
    language_options = user_settings.getLanguages();

    timer = app->root()->addChild(std::make_unique<Wt::WTimer>());
    timer->setInterval(std::chrono::milliseconds(750));
    timer->timeout().connect(this, &Trokam::SearchPage::timeout);

    input->setFocus();
}

void Trokam::SearchPage::search(
    const std::string &terms)
{
    std::string low_case_terms= Xapian::Unicode::tolower(terms);

    Xapian::doccount results_requested = 24;

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

    // std::vector<std::string> language_selected;
    // If no language selected, then use English.
    if(language_selected.size() == 0)
    {
        language_selected.push_back("english");
    }

    shared_resources->getNewDB();

    items_found =
        shared_resources->
            readable_content_db.search(
                low_case_terms,
                language_selected,
                results_requested);

    createFooter(container);
}

void Trokam::SearchPage::show_search_results()
{
    int total_results = items_found.size();
    auto dv = std::div(total_results, results_per_page);
    int total_pages = dv.quot;
    if(dv.rem > 0)
    {
        total_pages++;
    }

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

    if(w_footer == nullptr)
    {
        w_footer = base->addWidget(std::make_unique<Wt::WContainerWidget>());
    }

    w_footer->clear();

    int total_results = items_found.size();
    if(total_results == 0)
    {
        return;
    }

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

void Trokam::SearchPage::showUserOptions()
{
    auto preferences_box = addChild(std::make_unique<Wt::WDialog>("Preferences"));

    auto language_choices = std::make_unique<Wt::WTable>();
    language_choices->addStyleClass("table");

    const int max_per_column = 7;
    for(unsigned int i=0; i<language_options.size(); i++)
    {
        std::string language_name = Trokam::Preferences::languageName(i);
        bool language_selected = std::get<bool>(language_options[i]);

        int col = i / max_per_column;
        int row = i % max_per_column;

        Wt::WCheckBox *cb =
            language_choices->elementAt(row, col)->addNew<Wt::WCheckBox>(language_name);
        cb->setInline(false);
        cb->setChecked(language_selected);

        cb->checked().connect(  [=] { std::get<bool>(language_options[i])= true; });
        cb->unChecked().connect([=] { std::get<bool>(language_options[i])= false; });
    }

    // Show analysis option
    auto wt_show_analysis = std::make_unique<Wt::WCheckBox>("Show analysis");
    wt_show_analysis->setInline(false);
    wt_show_analysis->setChecked(true);

    auto w_theme = std::make_unique<Wt::WContainerWidget>();

    group = std::make_shared<Wt::WButtonGroup>();
    Wt::WRadioButton *button;

    button = w_theme->addNew<Wt::WRadioButton>("Light");
    button->setInline(false);
    group->addButton(button);

    button = w_theme->addNew<Wt::WRadioButton>("Dark");
    button->setInline(false);
    group->addButton(button);
    group->setSelectedButtonIndex(user_settings.getTheme());

    auto tabW = std::make_unique<Wt::WTabWidget>();

    tabW.get()->
        addTab(
            std::move(language_choices),
            "Languages",
            Wt::ContentLoading::Eager);

    tabW.get()->
        addTab(
            std::move(wt_show_analysis),
            "Analysis",
            Wt::ContentLoading::Eager);

    tabW.get()->
        addTab(
            std::move(w_theme),
            "Theme",
            Wt::ContentLoading::Eager);

    tabW.get()->setStyleClass("tabwidget");

    auto closeButton = std::make_unique<Wt::WPushButton>("Close");
    closeButton->addStyleClass("btn btn-primary");
    closeButton->clicked().connect([=] {
                                            if(savePreferences())
                                            {
                                                removeChild(preferences_box);
                                            }
                                       });

    // Process the dialog result.
    preferences_box->finished().connect([=] {
                                            if(savePreferences())
                                            {
                                                removeChild(preferences_box);
                                            }
                                        });

    auto explanation = std::make_unique<Wt::WLabel>("Reload Trokam page after changing your settings.");

    preferences_box->contents()->addWidget(std::move(tabW));
    preferences_box->footer()->addWidget(std::move(explanation));
    preferences_box->footer()->addWidget(std::move(closeButton));
    preferences_box->rejectWhenEscapePressed();
    preferences_box->setModal(false);
    preferences_box->show();
}

bool Trokam::SearchPage::savePreferences()
{
    Wt::log("info") << "group->selectedButtonIndex():" << group->selectedButtonIndex();
    unsigned int theme = group->selectedButtonIndex();
    user_settings.setTheme(theme);
    user_settings.setLanguages(language_options);
    std::string cookie_preferences = user_settings.serialize();
    application->setCookie("preferences", cookie_preferences, 10000000);
    return true;
}

void Trokam::SearchPage::inputKeyWentUp(
    const Wt::WKeyEvent &kEvent)
{
    if(kEvent.key() == Wt::Key::Down)
    {
        if(w_sugggestion_box != nullptr)
        {
            w_sugggestion_box->setHidden(false);
            w_sugggestion_box->setFocus();
            w_sugggestion_box->setCurrentIndex(0);
        }
    }
}

void Trokam::SearchPage::suggestionBoxKeyWentUp(
    const Wt::WKeyEvent &kEvent)
{
    if((kEvent.key() == Wt::Key::Up) && (w_sugggestion_box->currentIndex() == 0))
    {
        input->setFocus();
    }
}

void Trokam::SearchPage::suggestionBoxEnterPressed()
{
    std::string partial_input = input->text().toUTF8();

    std::vector<std::string> parts =
        Trokam::PlainTextProcessor::tokenize(partial_input);

    unsigned int total = parts.size();
    unsigned int last = total - 1;

    std::string user_input;
    for(unsigned int i=0; i<last; i++)
    {
        user_input += parts[i] + " ";
    }
    user_input += w_sugggestion_box->currentText().toUTF8();
    user_input = Xapian::Unicode::tolower(user_input);
    const std::string encoded_terms = Wt::Utils::urlEncode(user_input);
    input->setText(user_input);
    input->setFocus();

    destroySuggestionBox();

    std::string internal_url = "/";
    internal_url+= encoded_terms;
    application->setInternalPath(internal_url, true);
}

void Trokam::SearchPage::timeout()
{
    timer->stop();
    showSuggestions();
}

void Trokam::SearchPage::textInput()
{
    timer->start();
}

void Trokam::SearchPage::showSuggestions()
{
    std::string user_input= input->text().toUTF8();
    if(user_input.length() <= 3)
    {
        return;
    }

    std::vector<std::string> parts =
        Trokam::PlainTextProcessor::tokenize(user_input);

    unsigned int total = parts.size();
    unsigned int last = total - 1;

    if(parts[last].size() <= 3)
    {
        return;
    }

    user_input = parts[last];
    std::string low_case_terms= Xapian::Unicode::tolower(user_input);

    shared_resources->getNewDB();

    std::vector<std::pair<std::string, Xapian::doccount>> words_found =
        shared_resources->
            readable_content_db.lookUp(
                low_case_terms);

    if(words_found.size() > 0)
    {
        /**
         * DESIGN REVIEW
         * w_sugggestion_box may not be created in createSuggestionBox()
         * and the next line will fail.
         **/
        createSuggestionBox();
        w_sugggestion_box->clear();

        int count = 0;
        for(unsigned int i=0; i<words_found.size(); i++)
        {
            w_sugggestion_box->addItem(std::get<std::string>(words_found[i]));
            count++;
            if(count > 12)
            {
                break;
            }
        }
    }
}

void Trokam::SearchPage::createSuggestionBox()
{
    if(w_sugggestion_box == nullptr)
    {
        w_sugggestion_box = container->addNew<Wt::WSelectionBox>();
        w_sugggestion_box->setHidden(false);
        w_sugggestion_box->setMaximumSize(500, 600);
        w_sugggestion_box->positionAt(input);
        w_sugggestion_box->keyWentUp().connect(
            this, &Trokam::SearchPage::suggestionBoxKeyWentUp);
        w_sugggestion_box->enterPressed().connect(
            this, &Trokam::SearchPage::suggestionBoxEnterPressed);
        w_sugggestion_box->clicked().connect(
            this, &Trokam::SearchPage::suggestionBoxEnterPressed);
    }
}

void Trokam::SearchPage::destroySuggestionBox()
{
    if(w_sugggestion_box != nullptr)
    {
        container->removeWidget(w_sugggestion_box);
        w_sugggestion_box = nullptr;
    }
}

bool Trokam::SearchPage::isAgentMobile()
{
    const Wt::WEnvironment& env = Wt::WApplication::instance()->environment();
    std::string user_agent = env.headerValue("User-Agent");

    size_t pos =
        Trokam::PlainTextProcessor::caseInsensitiveFind(user_agent, "android");

    if(pos != std::string::npos)
    {
        return true;
    }

    if(env.agentIsMobileWebKit())
    {
        return true;
    }

    // It seems that the User-Agent is a desktop browser.
    return false;
}
