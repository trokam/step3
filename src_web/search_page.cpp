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
#include <cctype>
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
#include <Wt/WText.h>
#include <Wt/Utils.h>

// Xapian
#include <xapian.h>

// Trokam
#include "file_ops.h"
#include "plain_text_processor.h"
#include "search_page.h"
#include "shared_resources.h"

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

Trokam::SearchPage::SearchPage(
    boost::shared_ptr<Trokam::SharedResources> &sr):
        Wt::WContainerWidget(),
        shared_resources(sr)
{
    application = Wt::WApplication::instance();
    const Wt::WEnvironment& env = application->environment();
    auth_token = shared_resources->getAuthToken();

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
            shared_resources->insertOccurrence();
            std::string internal_path= application->internalPath();
            boost::algorithm::trim_if(
                internal_path, boost::algorithm::is_any_of("/ \n\r\t\\\""));
            const std::string search_terms=
                Wt::Utils::urlDecode(internal_path);
            search(search_terms);
            showSearchResults();
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
            boost::algorithm::trim_if(user_input, boost::algorithm::is_any_of(" \n\r\t\\"));
            searching = true;
            if(user_input.empty())
            {
                searching = false;
            }
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
        text_clipping = 700;
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
            boost::algorithm::trim_if(user_input, boost::algorithm::is_any_of(" \n\r\t\\"));
            searching = true;
            if(user_input.empty())
            {
                searching = false;
            }
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

    auto w_popup_info =
        std::make_unique<Wt::WPopupMenu>();

    // Create some menu items for the popup menu
    w_popup_info->addItem("About")->setLink(Wt::WLink("/info/about"));
    w_popup_info->addItem("News")->setLink(Wt::WLink("/info/news"));

    w_info = header->bindWidget(
        "button_info",
        std::make_unique<Wt::WPushButton>());
    w_info->addStyleClass("paging-button");
    w_info->setTextFormat(Wt::TextFormat::XHTML);
    w_info->setText("<span class=\"paging-text\">Info</span>");
    w_info->setMenu(std::move(w_popup_info));

    w_answer = container->addNew<Wt::WTemplate>();

    w_progress_bar = container->addNew<Wt::WProgressBar>();
    w_progress_bar->addStyleClass("w-100");
    w_progress_bar->setFormat("generating answer: %.0f %%");
    w_progress_bar->setMargin(10);
    w_progress_bar->setRange(0, 60);
    w_progress_bar->setHidden(true);

    w_button_clipping = container->addNew<Wt::WPushButton>();
    w_button_clipping->addStyleClass("paging-button");
    w_button_clipping->setTextFormat(Wt::TextFormat::XHTML);
    w_button_clipping->setText("<span class=\"paging-text\">show more</span>");
    w_button_clipping->setHidden(true);
    w_button_clipping->clicked().connect(this, &Trokam::SearchPage::showAnswer);

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

    timer = application->root()->addChild(std::make_unique<Wt::WTimer>());
    timer->setInterval(std::chrono::milliseconds(750));
    timer->timeout().connect(this, &Trokam::SearchPage::timeout);

    input->setFocus();

    // Initialize curl.
    curl = curl_easy_init();

    // training();
    checkTraining();
}

Trokam::SearchPage::~SearchPage()
{
    if(curl != NULL)
    {
        curl_easy_cleanup(curl);
    }
}

void Trokam::SearchPage::search(
    const std::string &terms)
{
    if(terms.empty())
    {
        items_found.clear();
        searching = false;
        return;
    }

    std::string low_case_terms= Xapian::Unicode::tolower(terms);

    auto [words, question] =
        Trokam::PlainTextProcessor::getQueryParts(low_case_terms);

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
                words,
                language_selected,
                results_requested);

    createFooter(container);

    if(ai_task.joinable())
    {
        ai_task.join();
    }

    if(!question.empty())
    {
        w_answer->setTemplateText("", Wt::TextFormat::UnsafeXHTML);
        w_button_clipping->setHidden(true);
        showing_complete_answer = true;

        answer = shared_resources->getAnswer(terms);
        if(!answer.empty())
        {
            showAnswer();
            return;
        }

        application->enableUpdates(true);
        waiting_for_answer = true;

        w_progress_bar->setHidden(false);
        ai_task =
            std::thread(
                &Trokam::SearchPage::getAiAnswer, this, application, question);

        std::thread show_progress(
            &Trokam::SearchPage::waitingTick, this, application);

        show_progress.detach();
    }
    else
    {
        w_answer->setTemplateText("", Wt::TextFormat::UnsafeXHTML);
        w_button_clipping->setHidden(true);
        showing_complete_answer = true;
    }
}

// void Trokam::SearchPage::getAiAnswer()
void Trokam::SearchPage::getAiAnswer(Wt::WApplication *app, const std::string &terms)
{
    answer = exec_post(terms);
    waiting_for_answer = false;

    Wt::WApplication::UpdateLock uiLock(app);
    if(uiLock)
    {
        w_progress_bar->setValue(0);
        w_progress_bar->setHidden(true);

        shared_resources->saveQA(terms, answer);
        showAnswer();
        app->triggerUpdate();
        app->enableUpdates(false);
    }
    else
    {
        Wt::log("error") << "failed getting uiLock in SearchPage::getAiAnswer(..)";
    }
}

void Trokam::SearchPage::waitingTick(Wt::WApplication *app)
{
    int i=0;
    while((i<100) && waiting_for_answer)
    {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        Wt::WApplication::UpdateLock uiLock(app);
        if(uiLock)
        {
            w_progress_bar->setValue(i);
            app->triggerUpdate();
        }
        i++;
    }

    if((i >= 100) && waiting_for_answer)
    {
        waiting_for_answer = false;
        application->enableUpdates(false);
        w_progress_bar->setValue(0);
        w_progress_bar->setHidden(true);
        application->refresh();
    }
}

void Trokam::SearchPage::showSearchResults()
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

            out=+ "<br/>";
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
            // out+= "</p>";

            auto oneRow = std::make_unique<Wt::WTemplate>();
            oneRow->setTemplateText(out, Wt::TextFormat::UnsafeXHTML);
            userFindings->elementAt(i, 0)->addWidget(std::move(oneRow));
        }

        std::string out= "<br/>";
        auto oneRow = std::make_unique<Wt::WTemplate>();
        oneRow->setTemplateText(out, Wt::TextFormat::UnsafeXHTML);
        userFindings->elementAt(end, 0)->addWidget(std::move(oneRow));
    }
    else if((items_found.size() == 0) && searching && !waiting_for_answer)
    {
        // Tell the user that there are not any results found.
        std::string msg;
        msg += "&nbsp;</br>";
        msg += "<h3 style=\"text-align:left\">No results were found.</h3>";
        msg += "<h3 style=\"text-align:left\">&bull; Check term spelling. Try similar concepts and synonyms if possible.</h3>";
        msg += "<h3 style=\"text-align:left\">&bull; Verify your preferences. You are searching for results in ";
        msg += getLanguagesSelected() + ".</h3>";
        // msg += "<h3 style=\"text-align:left\">&bull; <a href=\"/info/about#enhance-trokam\"</a>Contribute to enhancing Trokam.</h3>";

        userFindings->clear();
        w_answer->setTemplateText("", Wt::TextFormat::UnsafeXHTML);
        auto no_results_found = std::make_unique<Wt::WTemplate>();
        no_results_found->setTemplateText(msg, Wt::TextFormat::UnsafeXHTML);
        userFindings->elementAt(0, 0)->addWidget(std::move(no_results_found));
    }
    else
    {
        // Clear the table of findings and the answer.
        userFindings->clear();
        w_answer->setTemplateText("", Wt::TextFormat::UnsafeXHTML);
        w_footer->clear();
        searching = false;
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
            if(current_page > 1)
            {
                current_page--;
                showSearchResults();
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
            showSearchResults();
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
            showSearchResults();
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
            if(current_page < total_pages)
            {
                current_page++;
                showSearchResults();
                createFooter(base);
            }
        });

    std::string msg;
    msg += "&nbsp;</br>&nbsp;</br>";
    msg += "<h3 style=\"text-align:left\">Did you find it? In case you didn't:</h3>";
    msg += "<h3 style=\"text-align:left\">&bull; Check term spelling. Try similar concepts and synonyms if possible.</h3>";
    msg += "<h3 style=\"text-align:left\">&bull; Verify your preferences. You are searching for results in ";
    msg += getLanguagesSelected() + ".</h3>";

    auto did_you_find_it = std::make_unique<Wt::WTemplate>();
    did_you_find_it->setTemplateText(msg, Wt::TextFormat::UnsafeXHTML);
    container_l1->addWidget(std::move(did_you_find_it));
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

void Trokam::SearchPage::checkTraining()
{
    const Wt::WEnvironment& env = Wt::WApplication::instance()->environment();
    const std::string *training_value = env.getParameter("training");
    const std::string *pass_value = env.getParameter("pass");
    if(training_value && pass_value)
    {
        const std::string correct_pass = shared_resources->getPassword();
        if((*training_value == "true") && (*pass_value == correct_pass))
        {
            std::thread t(&Trokam::SearchPage::training, this);
            t.detach();
        }
    }
}

void Trokam::SearchPage::training()
{
    const std::string words_path = "/usr/local/etc/trokam/words.json";
    std::string words_content = Trokam::FileOps::read(words_path);
    nlohmann::json words_json = nlohmann::json::parse(words_content);

    std::map<std::string, std::vector<std::string>> words =
        words_json["words"].get<std::map<std::string, std::vector<std::string>>>();

    for(auto iter= words.begin(); iter != words.end(); ++iter)
    {
        const std::string language = iter->first;
        const std::vector<std::string> term_collection = iter->second;
        for(const auto &term: term_collection)
        {
            Wt::log("info") << "language:" << language << " -- term:" << term;

            std::string low_case_terms= Xapian::Unicode::tolower(term);

            Xapian::doccount results_requested = 2;

            std::vector<std::string> language_selected;
            language_selected.push_back(language);

            shared_resources->getNewDB();

            shared_resources->
                readable_content_db.search(
                    low_case_terms,
                    language_selected,
                    results_requested);
        }
    }
}

std::string Trokam::SearchPage::getLanguagesSelected()
{
    std::vector<std::string> languages;
    for(unsigned int i=0; i<language_options.size(); i++)
    {
        if(std::get<bool>(language_options[i]))
        {
            std::string language_name = Trokam::Preferences::languageName(i);
            // Convert the fist letter of language_name to uppercase.
            std::string language_formal;
            language_formal += std::toupper(language_name[0]);
            language_formal += language_name.substr(1);
            languages.push_back(language_formal);
        }
    }

    if(languages.size() == 0)
    {
        Wt::log("error") << "no language selected, assuming english.";
        int index_english = Trokam::Language::ENGLISH;
        std::string result = Trokam::Preferences::languageName(index_english);
        return result;
    }
    else if(languages.size() == 1)
    {
        std::string result = languages[0];
        return result;
    }
    else if(languages.size() == 2)
    {
        std::string result = languages[0] + " and " + languages[1];
        return result;
    }
    else
    {
        std::string result = languages[0];
        for(unsigned int i=1; i<(languages.size()-1); i++)
        {
            result += ", " + languages[i];
        }
        result += " and " + languages[languages.size()-1];
        return result;
    }
}

std::string Trokam::SearchPage::exec_post(const std::string &terms)
{
    std::string raw;
    struct curl_slist *list = NULL;
    std::vector<std::string> headers;

    std::string header_0 = "Content-Type: application/json";
    headers.push_back(header_0);

    std::string header_1 = "Authorization: Bearer " + auth_token;
    headers.push_back(header_1);

    for(size_t i=0; i<headers.size(); i++)
    {
        list = curl_slist_append(list, headers[i].c_str());
    }

    std::vector<nlohmann::json> messages;

    nlohmann::json msg0;
    msg0["content"] = "You are a helpful assistant.";
    msg0["role"] = "system";
    messages.push_back(msg0);

    nlohmann::json msg1;
    msg1["content"] = terms;
    msg1["role"] = "user";
    messages.push_back(msg1);

    nlohmann::json data;
    data["model"] = "gpt-3.5-turbo";
    data["messages"] = messages;

    std::string payload = data.dump();

    std::string url= "https://api.openai.com/v1/chat/completions";
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, payload.length());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, appendData);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &raw);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 1000L);

    std::string answer;

    /* Perform the request, res will get the return code */
    CURLcode res = curl_easy_perform(curl);
    if(res != CURLE_OK)
    {
        Wt::log("info") << "libcurl fail:" << curl_easy_strerror(res);
        return answer;
    }

    if(list != NULL)
    {
        curl_slist_free_all(list);
    }

    try
    {
        nlohmann::json reply = nlohmann::json::parse(raw);
        for(auto& elem : reply["choices"])
        {
            answer = elem["message"]["content"];
        }
    }
    catch(const std::exception& e)
    {
        Wt::log("error") << "fail processing reply from ai reply.";
        Wt::log("error") << e.what();
        return "";
    }

    return answer;
}

void Trokam::SearchPage::showAnswer()
{
    std::string out;

    if(answer.length() > text_clipping)
    {
        if(showing_complete_answer)
        {
            size_t loc= answer.find("\n\n", text_clipping);
            if(loc != std::string::npos)
            {
                std::string partial = answer.substr(0, loc);
                boost::replace_all(partial, "\n\n", "<br/><br/>");
                partial += " [..]";
                out+= "<span class=\"snippet\">" + partial + "</strong></span><br/>";
                out+= "<br/>";
            }
            else
            {
                std::string partial = answer.substr(0, text_clipping);
                boost::replace_all(partial, "\n\n", "<br/><br/>");
                partial += " [..]";
                out+= "<span class=\"snippet\">" + partial + "</strong></span><br/>";
                out+= "<br/>";
            }
            w_button_clipping->setText("<span class=\"paging-text\">show more</span>");
            w_button_clipping->setHidden(false);
            showing_complete_answer = false;
        }
        else
        {
            boost::replace_all(answer, "\n\n", "<br/><br/>");
            out+= "<span class=\"snippet\">" + answer + "</strong></span><br/>";
            out+= "<br/>";
            w_button_clipping->setText("<span class=\"paging-text\">show less</span>");
            w_button_clipping->setHidden(false);
            showing_complete_answer = true;
        }
    }
    else
    {
        boost::replace_all(answer, "\n\n", "<br/><br/>");
        out+= "<span class=\"snippet\">" + answer + "</strong></span><br/>";
        out+= "<br/>";
        w_button_clipping->setHidden(true);
    }
    w_answer->setTemplateText(out, Wt::TextFormat::UnsafeXHTML);
}
