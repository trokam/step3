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
#include <Wt/WButtonGroup.h>
#include <Wt/WCheckBox.h>
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
#include <Wt/WRadioButton.h>
#include <Wt/WString.h>
// #include <Wt/WVBoxLayout.h>
#include <Wt/WStringListModel.h>
#include <Wt/WTabWidget.h>
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
#include "plain_text_processor.h"
#include "searchPage.h"
#include "sharedResources.h"

// #define INLINE_JAVASCRIPT(...) #__VA_ARGS__

Trokam::SearchPage::SearchPage(
    boost::shared_ptr<Trokam::SharedResources> &sr,
    Wt::WApplication* app):
        Wt::WContainerWidget(),
        application(app),
        shared_resources(sr)
{
    Wt::log("info") << "SearchPage constructor";

    const Wt::WEnvironment& env = Wt::WApplication::instance()->environment();

    std::string cookie_preferences;
    if(env.getCookie("preferences"))
    {
        cookie_preferences= *(env.getCookie("preferences"));
        Wt::log("info") << "createApplication -- cookie -- preferences:" << cookie_preferences;
    }
    else
    {
        Wt::log("info") << "cookie -- preferences = NULL";
    }

    user_settings.generate(cookie_preferences);
    Wt::log("info") << "user_settings.getTheme()=" << user_settings.getTheme();

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
    // input = container->addWidget(std::make_unique<Wt::WLineEdit>());
    // input->addStyleClass("form-control");
    input->setPlaceholderText("Search for ...");
    input->enterPressed().connect(
        [=] {
            w_sugggestion_box->setHidden(true);
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

    input->keyWentUp().connect(this, &Trokam::SearchPage::inputKeyWentUp);

    /*
    input->keyWentUp().connect(
        [=] {
            std::string user_input= input->text().toUTF8();
            if(user_input.length() <= 3)
            {
                w_sugggestion_box->setHidden(true);
                return;
            }

            w_sugggestion_box->setHidden(false);

            Wt::log("info") << "----- user_input:" << user_input;
            std::string low_case_terms= Xapian::Unicode::tolower(user_input);

            // Xapian::doccount results_requested = 24;

            shared_resources->getNewDB();

            std::vector<std::pair<std::string, Xapian::doccount>> words_found =
                shared_resources->
                    readable_content_db.lookUp(
                        low_case_terms);

            int count = 0;
            for(unsigned int i=0; i<words_found.size(); i++)
            {
                Wt::log("info") << "===== suggestion:"
                                << std::get<std::string>(words_found[i]);
                count++;
                if(count > 10)
                {
                    break;
                }
            }
        });
    */

    w_sugggestion_box = container->addNew<Wt::WSelectionBox>();
    // w_sugggestion_box->setHidden(true);
    // w_sugggestion_box->setPositionScheme(Wt::PositionScheme::Absolute);
    // w_sugggestion_box->setPositionScheme(Wt::PositionScheme::Relative);
    // w_sugggestion_box->setOffsets(150, Wt::Side::Left);
    w_sugggestion_box->setMaximumSize(500, 600);
    // w_sugggestion_box->setPopup(true);
    w_sugggestion_box->positionAt(input);
    // w_sugggestion_box->addItem("Heavy");
    // w_sugggestion_box->addItem("Medium");
    // w_sugggestion_box->addItem("Light");
    // w_sugggestion_box->setCurrentIndex(1); // Select 'medium' by default.
    // w_sugggestion_box->setMargin(10, Wt::Side::Right);
    w_sugggestion_box->keyWentUp().connect(
        this, &Trokam::SearchPage::suggestionBoxKeyWentUp);
    w_sugggestion_box->enterPressed().connect(
        this, &Trokam::SearchPage::suggestionBoxEnterPressed);
    w_sugggestion_box->escapePressed().connect(
        this, &Trokam::SearchPage::suggestionBoxEscapePressed);

    auto button = header->bindWidget(
        "button",
        std::make_unique<Wt::WPushButton>("search"));
    button->addStyleClass("btn");
    button->addStyleClass("btn-outline-secondary");
    button->clicked().connect(
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

    w_button_preferences = header->bindWidget(
        "button_preferences",
        std::make_unique<Wt::WPushButton>());
    w_button_preferences->addStyleClass("paging-button");
    w_button_preferences->setTextFormat(Wt::TextFormat::XHTML);
    w_button_preferences->setText("<span class=\"paging-text\">Preferences</span>");
    w_button_preferences->
        clicked().connect(this, &Trokam::SearchPage::showLanguageOptions);

    w_about = header->bindWidget(
        "button_about",
        std::make_unique<Wt::WPushButton>());
    w_about->addStyleClass("paging-button");
    w_about->setTextFormat(Wt::TextFormat::XHTML);
    w_about->setText("<span class=\"paging-text\">About</span>");

    userFindings =
        container->addWidget(std::make_unique<Wt::WTable>());

    /*
    container->addWidget(
        std::make_unique<Wt::WTemplate>(
            Wt::WString::tr("search-page-footer")));
    */

    // serverSideFilteringPopups(container);
    createFooter(container);

    /**
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
    **/

   /**
     * Initialize searched languages.
     **/
    for(unsigned int i=0; i<LANGUAGES_TOTAL; i++)
    {
        auto language_item = std::make_pair(i, false);
        language_options.push_back(language_item);
    }
    language_options = user_settings.getLanguages();

    w_sugggestion_box->setHidden(true);

    Wt::log("info") << "current path:" << application->internalPath();
}

void Trokam::SearchPage::search(
    const std::string &terms)
{
    Wt::log("info") << "+++++++ search() -- terms:" << terms;

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

    Wt::log("info") << "+++++++ total results:" << items_found.size();
    createFooter(container);

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

/**
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
**/

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

void Trokam::SearchPage::showLanguageOptions()
{
    /**
    std::string cookie_preferences;

    const Wt::WEnvironment& env = application->environment();
    if(env.getCookie("preferences"))
    {
        cookie_preferences= *(env.getCookie("preferences"));
        Wt::log("info") << "showLanguageOptions -- cookie -- preferences:" << cookie_preferences;
    }
    else
    {
        Wt::log("info") << "cookie -- preferences = NULL";
    }

    preferences.generate(cookie_preferences);
    language_options = preferences.getLanguages();
    **/

    /**
    for(unsigned int i=0; i<language_options.size(); i++)
    {
        std::cout << "--- element:" << i << " -- " << std::get<bool>(language_options[i]) << '\n';
    }
    **/

    auto preferences_box = addChild(std::make_unique<Wt::WDialog>("Preferences"));
    // auto header = std::make_unique<Wt::WText>(Wt::WString("Search Languages"));

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

    // wt_show_analysis->checked().connect(  [=] { std::get<bool>(language_options[i])= true; });
    // wt_show_analysis->unChecked().connect([=] { std::get<bool>(language_options[i])= false; });

    /*
    auto change_style = std::make_unique<Wt::WPushButton>("Change style");
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
    */

            // const Wt::WEnvironment& env = application->environment();
            // int page_style = 0;
            /*
            if(env.getCookie("theme") != nullptr)
            {
                page_style= *(env.getCookie("theme"));
            }
            */

auto w_theme = std::make_unique<Wt::WContainerWidget>();

group = std::make_shared<Wt::WButtonGroup>();
Wt::WRadioButton *button;

button = w_theme->addNew<Wt::WRadioButton>("Light");
button->setInline(false);
group->addButton(button);

button = w_theme->addNew<Wt::WRadioButton>("Dark");
button->setInline(false);
group->addButton(button);

Wt::log("info") << "user_settings.getTheme()=" << user_settings.getTheme();
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

    /*
    tabW.get()->addTab(std::make_unique<Wt::WTextArea>("You could change any other style attribute of the"
                                " tab widget by modifying the style class."
                                " The style class 'trhead' is applied to this tab."),
                "Style", Wt::ContentLoading::Eager)->setStyleClass("trhead");
    */

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

    // preferences_box->titleBar()->addWidget(std::move(header));
    // preferences_box->contents()->addWidget(std::move(language_choices));
    preferences_box->contents()->addWidget(std::move(tabW));
    preferences_box->footer()->addWidget(std::move(closeButton));
    preferences_box->rejectWhenEscapePressed();
    preferences_box->setModal(false);
    preferences_box->show();
}

bool Trokam::SearchPage::savePreferences()
{
    /**
    std::string language_selected;
    for(unsigned int i=0; i<language_options.size(); i++)
    {
        if(std::get<bool>(language_options[i]))
        {
            language_selected += std::to_string(i) + ",";
        }
    }

    application->setCookie("languages", language_selected, 10000000);
    **/
    // application->setCookie("theme", group->selectedButtonIndex(), 10000000);

    Wt::log("info") << "group->selectedButtonIndex():" << group->selectedButtonIndex();
    unsigned int theme = group->selectedButtonIndex();
    user_settings.setTheme(theme);
    user_settings.setLanguages(language_options);

    std::string cookie_preferences = user_settings.serialize();
    Wt::log("info") << "cookie_preferences:" << cookie_preferences;
    application->setCookie("preferences", cookie_preferences, 10000000);

    return true;
}

/**
Wt::WSuggestionPopup* Trokam::SearchPage::createAliasesMatchingPopup(
    WContainerWidget *parent)
{
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
*/

void Trokam::SearchPage::inputKeyWentUp(
    const Wt::WKeyEvent &kEvent)
{
    std::string user_input= input->text().toUTF8();
    if(user_input.length() <= 3)
    {
        w_sugggestion_box->setHidden(true);
        return;
    }

    if((kEvent.key() == Wt::Key::Down) && (w_sugggestion_box->isVisible()))
    {
        w_sugggestion_box->setFocus();
        w_sugggestion_box->setCurrentIndex(0);
    }
    if(kEvent.key() == Wt::Key::Enter)
    {
        return;
    }
    else
    {
        std::vector<std::string> parts =
            Trokam::PlainTextProcessor::tokenize(user_input);

        unsigned int total = parts.size();
        unsigned int last = total - 1;

        for(unsigned int i=0; i<parts.size(); i++)
        {
            Wt::log("info") << "parts[" << i << "]=" << parts[i];
        }

        if(parts[last].size() <= 3)
        {
            w_sugggestion_box->setHidden(true);
            return;
        }

        user_input = parts[last];

        w_sugggestion_box->setHidden(false);
        w_sugggestion_box->positionAt(input);

        Wt::log("info") << "----- user_input:" << user_input;
        std::string low_case_terms= Xapian::Unicode::tolower(user_input);

        // Xapian::doccount results_requested = 24;

        shared_resources->getNewDB();

        std::vector<std::pair<std::string, Xapian::doccount>> words_found =
            shared_resources->
                readable_content_db.lookUp(
                    low_case_terms);

        w_sugggestion_box->clear();

        int count = 0;
        for(unsigned int i=0; i<words_found.size(); i++)
        {
            Wt::log("info") << "===== suggestion:"
                            << std::get<std::string>(words_found[i]);

            w_sugggestion_box->addItem(std::get<std::string>(words_found[i]));

            count++;
            if(count > 12)
            {
                break;
            }
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

    Wt::log("info") << "++++ searching for:" << user_input;

    user_input = Xapian::Unicode::tolower(user_input);
    const std::string encoded_terms = Wt::Utils::urlEncode(user_input);
    input->setText(user_input);
    input->setFocus();
    w_sugggestion_box->setHidden(true);

    std::string internal_url = "/";
    internal_url+= encoded_terms;
    application->setInternalPath(internal_url, true);
}

void Trokam::SearchPage::suggestionBoxEscapePressed()
{
    input->setFocus();
    w_sugggestion_box->setHidden(true);
}