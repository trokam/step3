/***********************************************************************
 *                            T R O K A M
 *                       Internet Search Engine
 *
 * Copyright (C) 2017, Nicolas Slusarenko
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
#include <memory>
#include <cmath>
#include <cstdlib>

// Boost
#include <boost/algorithm/string.hpp>
#include <boost/thread.hpp>

// Wt
#include <Wt/WAnchor.h>
#include <Wt/WCheckBox.h>
#include <Wt/WComboBox.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WDialog.h>
#include <Wt/WEnvironment.h>
#include <Wt/WMenu.h>
#include <Wt/WLineEdit.h>
#include <Wt/WHBoxLayout.h>
#include <Wt/WProgressBar.h>
#include <Wt/WPushButton.h>
#include <Wt/WMessageBox.h>
#include <Wt/WStackedWidget.h>
#include <Wt/WVBoxLayout.h>
#include <Wt/WTabWidget.h>
#include <Wt/WTemplate.h>
#include <Wt/WText.h>
#include <Wt/WTextArea.h>

// Xapian
#include <xapian.h>

// Trokam
#include "bundle.h"
#include "common.h"
#include "deferredWidget.h"
#include "file_ops.h"
#include "searchWidget.h"
#include "plain_text_processor.h"
#include "preferences.h"

Trokam::SearchWidget::SearchWidget(
    boost::shared_ptr<Trokam::SharedResources> &sr,
    Wt::WApplication* app):
        PageWidget(sr, app), shared_resources(sr)
{
    phraseOnFocus= -1;

    addStyleClass("container-fluid");

    // setDbTimeOut();

    /**
     * Main AboutWidget
     **/
    std::unique_ptr<Wt::WVBoxLayout> vbox =
        std::make_unique<Wt::WVBoxLayout>();
    vbox->setPreferredImplementation(
        Wt::LayoutImplementation::JavaScript);

    /**
     * Information
     **/
    vbox->addWidget(
        std::make_unique<Wt::WTemplate>(
            Wt::WString::tr("general-info")));

    /**
     * Small logo, show after the first search.
     **/
    vbox->addWidget(
        std::make_unique<Wt::WTemplate>(
            Wt::WString::tr("small-logo")));

    /**
     * Big logo, shown on starting page.
     **/
    vbox->addWidget(
        std::make_unique<Wt::WTemplate>(
            Wt::WString::tr("big-logo")));

    /**
     * The box where the user enters searching terms.
     **/
    auto entrance =
        std::make_unique<Wt::WTemplate>(Wt::WString::tr("search-box"));
    userInput =
        entrance->bindWidget(
            "input-entrance", std::make_unique<Wt::WLineEdit>());
    userInput->setPlaceholderText("Search for ...");
    userInput->keyWentDown().connect(
        this, &Trokam::SearchWidget::keyPressedEntrance);
    userInput->textInput().connect(
        this, &Trokam::SearchWidget::textInput);
    vbox->addWidget(std::move(entrance));

    /**
     * Brief introduction.
     **/
    auto brief =
        std::make_unique<Wt::WTemplate>(
            Wt::WString::tr("brief-intro"));
    vbox->addWidget(std::move(brief));

    /**
     * Languages searched.
     **/
    auto lang_container =
        std::make_unique<Wt::WContainerWidget>();
    lang_container->setStyleClass("yellow-box");

    auto hbox =
        lang_container->
            setLayout(std::make_unique<Wt::WHBoxLayout>());

    auto item = std::make_unique<Wt::WText>("Item 1");
    item->setStyleClass("green-box");
    hbox->addWidget(std::move(item), 1);

    /*
    item = std::make_unique<Wt::WText>("Item 2");
    item->setStyleClass("blue-box");
    hbox->addWidget(std::move(item));
    */

    auto bt_lang_sel = std::make_unique<Wt::WPushButton>("language");
    // bt_lang_sel->setStyleClass("btn-success");
    bt_lang_sel->setStyleClass("btn-primary");
    // bt_lang_sel->setStyleClass("btn-outline-primary");
    bt_lang_sel.get()->
        // clicked().connect(bt_lang_sel.get(), &Wt::WPushButton::disable);
        clicked().connect(this, &Trokam::SearchWidget::showLanguageOptions);

    hbox->addWidget(std::move(bt_lang_sel));

    /*
    auto cb = std::make_unique<Wt::WComboBox>();
    cb->addItem("Heavy");
    cb->addItem("Medium");
    cb->addItem("Light");
    cb->setCurrentIndex(1); // Show 'Medium' initially.
    cb->setMargin(10, Wt::Side::Right);
    hbox->addWidget(std::move(cb));
    */

    vbox->addWidget(std::move(lang_container));

    /*
    auto cb = lang_container->addNew<Wt::WComboBox>();
    cb->addItem("Heavy");
    cb->addItem("Medium");
    cb->addItem("Light");
    cb->setCurrentIndex(1); // Show 'Medium' initially.
    cb->setMargin(10, Wt::Side::Right);
    vbox->addWidget(std::move(lang_container));
    */

    /**
     * Information about the results.
     **/
    auto infoResults =
        std::make_unique<Wt::WTemplate>(
            Wt::WString::tr("info-results"));
    vbox->addWidget(std::move(infoResults));

    /**
    std::string tpl_pagination =
        "<nav aria-label=\"Page navigation\">"
        "<ul class=\"pagination\">"
        "    <li>"
        "    <a href=\"#\" aria-label=\"Previous\">"
        "        <span aria-hidden=\"true\"><span aria-hidden=\"true\">&larr;</span> previous</span>"
        "    </a>"
        "    </li>"
        "    <li><a href=\"#\">1</a></li>"
        "    <li><a href=\"#\">2</a></li>"
        "    <li><a href=\"#\">3</a></li>"
        "    <li><a href=\"#\">4</a></li>"
        "    <li><a href=\"#\">5</a></li>"
        "    <li>"
        "    <a href=\"#\" aria-label=\"Next\">"
        "        <span aria-hidden=\"true\">next <span aria-hidden=\"true\">&rarr;</span></span>"
        "    </a>"
        "    </li>"
        "</ul>"
        "</nav>";

    auto wt_pagination =
        std::make_unique<Wt::WTemplate>(tpl_pagination);
    **/


    /*
    std::string tpl_pagination =
        "<nav aria-label=\"Page navigation\">"
        "<ul class=\"pagination\">"
        "    <li>${link_previous}</li>"
        "    <li><a href=\"#\">1</a></li>"
        "    <li><a href=\"#\">2</a></li>"
        "    <li><a href=\"#\">3</a></li>"
        "    <li><a href=\"#\">4</a></li>"
        "    <li><a href=\"#\">5</a></li>"
        "    <li>${link_next}</li>"
        "</ul>"
        "</nav>";
    */

    /*
    std::string tpl_pagination =
        "<nav aria-label=\"Page navigation\">"
        "<ul class=\"pagination\">"
        "    <li>${link_previous}</li>"
        "    <li>${link_group_1}</li>"
        "    <li>${link_group_2}</li>"
        "    <li><a href=\"#\">3</a></li>"
        "    <li><a href=\"#\">4</a></li>"
        "    <li><a href=\"#\">5</a></li>"
        "    <li>${link_next}</li>"
        "</ul>"
        "</nav>";

    Wt::WString inner_previous = "<span aria-hidden=\"true\"><span aria-hidden=\"true\">&larr;</span> previous</span>";
    auto wt_previous = std::make_unique<Wt::WAnchor>(Wt::WLink("#"), inner_previous);
    wt_previous->clicked().connect([=] { Wt::log("info") << "previous";});

    Wt::WString inner_next = "<span aria-hidden=\"true\">next <span aria-hidden=\"true\">&rarr;</span></span>";
    auto wt_next = std::make_unique<Wt::WAnchor>(Wt::WLink("#"), inner_next);
    wt_next->clicked().connect([=] { Wt::log("info") << "next"; });

    auto wt_group_1 = std::make_unique<Wt::WAnchor>(Wt::WLink("#"), "1");
    wt_group_1->clicked().connect([=] { Wt::log("info") << "group_1";});

    auto wt_group_2 = std::make_unique<Wt::WAnchor>(Wt::WLink("#"), "2");
    wt_group_2->clicked().connect([=] { Wt::log("info") << "group_2";});

    auto wt_pagination = std::make_unique<Wt::WTemplate>(tpl_pagination);

    wt_pagination->bindWidget("link_previous", std::move(wt_previous));
    wt_pagination->bindWidget("link_next", std::move(wt_next));
    wt_pagination->bindWidget("link_group_1", std::move(wt_group_1));
    wt_pagination->bindWidget("link_group_2", std::move(wt_group_2));
    */

//    int total_results = 26;
//    int results_per_page = 8;


    /**
     * The box on which the results are displayed.
     **/
    auto subStack = std::make_unique<Wt::WStackedWidget>();     // ??????????
    subStack->addStyleClass("contents");
    subStack->setOverflow(Wt::Overflow::Auto);

    auto findingsBox =
        std::make_unique<Wt::WTemplate>(
            Wt::WString::tr("findings-box"));
    userFindings =
        findingsBox->bindWidget(
            "user-findings", std::make_unique<Wt::WTable>());
    subStack->addWidget(std::move(findingsBox));
    // subStack->addWidget(std::move(wt_pagination));
    vbox->addWidget(std::move(subStack), 1);

    // vbox->addWidget(std::move(wt_pagination));
    // vbox->insertWidget(FOOTER, std::move(wt_pagination));

    // auto wt_footer = std::make_unique<Wt::WContainerWidget>();
    // wt_footer->addWidget(std::move(wt_pagination));
    // vbox->addWidget(std::move(wt_footer));

    // generateFooter();
    auto wt_pagination = std::make_unique<Wt::WTemplate>("");
    // ptr_pagination = vbox->insertWidget(FOOTER, std::move(wt_pagination));
    ptr_pagination = vbox->addWidget(std::move(wt_pagination));

    Wt::log("info") << "vbox count:" << vbox->count();
    // vbox->itemAt(count-1)->addWidget(std::move(wt_footer));

/*
    vbox->itemAt(GENERAL_INFO)->widget()->setHidden(false);
    vbox->itemAt(SMALL_LOGO)->widget()->setHidden(true);
    vbox->itemAt(BIG_LOGO)->widget()->setHidden(false);
    vbox->itemAt(BRIEF_INTRO)->widget()->setHidden(false);
    vbox->itemAt(LANG_SEARCHED)->widget()->setHidden(false);
    vbox->itemAt(SEARCH_STATE)->widget()->setHidden(true);
*/

    vbox->itemAt(GENERAL_INFO)->widget()->setHidden(false);
    vbox->itemAt(SMALL_LOGO)->widget()->setHidden(false);
    vbox->itemAt(BIG_LOGO)->widget()->setHidden(false);
    vbox->itemAt(BRIEF_INTRO)->widget()->setHidden(false);
    vbox->itemAt(LANG_SEARCHED)->widget()->setHidden(false);
    vbox->itemAt(SEARCH_STATE)->widget()->setHidden(false);

    setLayout(std::move(vbox));

    timer = app->root()->addChild(std::make_unique<Wt::WTimer>());
    timer->setInterval(std::chrono::milliseconds(750));
    timer->timeout().connect(this, &Trokam::SearchWidget::timeout);

    /**
     * Initialize searched languages.
     **/
    for(unsigned int i=0; i<LANGUAGES_TOTAL; i++)
    {
        auto language_item = std::make_pair(i, false);
        language_options.push_back(language_item);
    }

    const Wt::WEnvironment& env = Wt::WApplication::instance()->environment();

    std::string languages_selected= *env.getCookie("languages");
    Wt::log("info") << "cookie language=" << languages_selected;

    if(!languages_selected.empty())
    {
        std::vector<std::string> languages_slices =
            Trokam::PlainTextProcessor::tokenize(languages_selected, ',');

        for(const std::string &element: languages_slices)
        {
            unsigned int language_id = std::atoi(element.c_str());
            std::get<bool>(language_options[language_id]) = true;
        }
    }

    // Checks if at least one language is selected.
    bool is_language_selected = false;
    for(const auto &element: language_options)
    {
        is_language_selected |= std::get<bool>(element);
    }

    // TODO: If there is not cookie this set the default value to English
    //       But, setting the default language to the same one of the
    //       browser is better.
    if(!is_language_selected)
    {
        std::get<bool>(language_options[Trokam::Language::ENGLISH])= true;
    }

    /**
     * Set the focus on the user input box.
     **/
    userInput->setFocus(true);
}

Trokam::SearchWidget::~SearchWidget()
{
    if(timer)
    {
        delete timer;
    }
}

void Trokam::SearchWidget::populateSubMenu(Wt::WMenu *menu)
{}

void Trokam::SearchWidget::keyPressedEntrance(const Wt::WKeyEvent &kEvent)
{
    Wt::log("info") << __PRETTY_FUNCTION__;

    if(kEvent.key() == Wt::Key::Enter)
    {
        timer->stop();

        if(phrasesPopup)
        {
            phrasesPopup->setHidden(true);
        }

        layout()->itemAt(0)->widget()->setHidden(true);   /// General Info
        layout()->itemAt(1)->widget()->setHidden(false);  /// Small logo
        layout()->itemAt(2)->widget()->setHidden(true);   /// Big log
        layout()->itemAt(4)->widget()->setHidden(true);   /// Brief intro
        layout()->itemAt(SEARCH_STATE)->widget()->setHidden(true);

        application->processEvents();

        const std::string choice= userInput->text().toUTF8();
        search(choice);
        show_search_results();
        generateFooter();
    }

    if ((kEvent.key() == Wt::Key::Down) && (phrasesPopup))
    {
        userInput->setFocus(false);
        std::vector<Wt::WMenuItem*> alternatives= phrasesPopup->items();
        alternatives[0]->setFocus(true);
        phraseOnFocus= 0;
    }
}

void Trokam::SearchWidget::phrasesPopupKeyPressed(
    const Wt::WKeyEvent &kEvent)
{
    Wt::log("info") << __PRETTY_FUNCTION__;

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
    if(kEvent.key() == Wt::Key::Up)
    {
        phraseOnFocus--;
        if((0 <= phraseOnFocus) && (phraseOnFocus < phrasesPopup->count()))
        {
            std::vector<Wt::WMenuItem*> alternatives= phrasesPopup->items();
            alternatives[phraseOnFocus]->setFocus(true);
        }
        else if (phraseOnFocus == -1)
        {
            userInput->setFocus(true);
        }
    }
    if(kEvent.key() == Wt::Key::Enter)
    {
        timer->stop();
        if(phrasesPopup)
        {
            phrasesPopup->select(phraseOnFocus);
        }
    }
    else
    {
        Wt::log("info") << "other key";
    }
}

void Trokam::SearchWidget::timeout()
{
    timer->stop();
    searchForPhrases();
}

void Trokam::SearchWidget::textInput()
{
    Wt::log("info") << __PRETTY_FUNCTION__;
    timer->start();
}

void Trokam::SearchWidget::searchForPhrases()
{
    Wt::log("info") << __PRETTY_FUNCTION__;

    std::string prefix= userInput->text().toUTF8();
    boost::algorithm::to_lower(prefix);

    // setDbTimeOut(1);

    if(prefix.length() >= 2)
    {
        auto data =
            shared_resources->
                readable_content_db.lookUp(prefix);

        unsigned int max_results = 12; // opt.maxResults();

        /*
        unsigned int i=0;
        while((i<data.size()) && (i<max_results))
        {
            std::cout
                << "term:" << std::get<0>(data[i]) << '\t'
                << "occurrences:" << std::get<1>(data[i]) << '\n';
            i++;
        }
        */

        if(phrasesPopup)
        {
            delete phrasesPopup.release();
        }

        phrasesPopup = std::make_unique<Wt::WPopupMenu>();
        phrasesPopup->
            itemSelected().connect(
                this,
                &Trokam::SearchWidget::phrasesPopupSelect);

        for(size_t i= 0; ((i<data.size()) && (i<max_results)); i++)
        {
            phrasesPopup->addItem(std::get<0>(data[i]));
        }

        std::vector<Wt::WMenuItem*> alternatives= phrasesPopup->items();
        for(unsigned int i=0; i<alternatives.size(); i++)
        {
            alternatives[i]->setCanReceiveFocus(true);
            alternatives[i]->
                keyWentDown().connect(
                    this,
                    &SearchWidget::phrasesPopupKeyPressed);
        }

        phrasesPopup->popup(userInput);

        if(data.size() > 0)
        {
            phrasesPopup->setHidden(false);
        }
        else
        {
            phrasesPopup->setHidden(true);
        }
    }
    else
    {
        phrasesPopup->setHidden(true);
        userFindings->clear();
        layout()->itemAt(SEARCH_STATE)->widget()->setHidden(true);
    }
}

void Trokam::SearchWidget::getPhrases(const std::string &sentence,
                                      const int &dbId)
{
    /**

    Wt::log("info") << "getPhrases -- dbId: " << dbId;

    try
    {
        pqxx::result answer;
        resources->dbCluster[dbId]->execSql(sentence, answer);

        for (pqxx::result::iterator row = answer.begin(); row != answer.end(); row++)
        {
            Trokam::Sequence seq;
            seq.index= row[0].as(int());
            seq.value= row[1].as(std::string());
            seq.occurrences= row[2].as(int());

            bagPhrases.push(seq);
        }
    }
    catch(const std::exception &e)
    {
        Wt::log("info") << "error: " << e.what();
    }

    **/
}

void Trokam::SearchWidget::insertSequence(const Trokam::Sequence &seq)
{
    /**
     * Compare the text in the argument to each one of the texts
     * already stored. If a match is found then it is not included.
     **/

    /**
    for(std::vector<Trokam::Sequence>::iterator it= sequenceCollection.begin(); it!=sequenceCollection.end(); ++it)
    {
        if (it->value==seq.value)
        {
            if(it->occurrences > seq.occurrences)
            {
                return;
            }
            else
            {
                it->occurrences = seq.occurrences;
                return;
            }
        }
    }
    **/

    /**
     * Reaching here means that the text was not found in
     * the collection. A new one is inserted.
     **/
    /** sequenceCollection.push_back(seq); **/
}

void Trokam::SearchWidget::setDbTimeOut(const int &timeOutSeconds)
{
    /**

    Wt::log("info") << __PRETTY_FUNCTION__;

    std::string statement;
    statement = "SET statement_timeout = " + std::to_string(timeOutSeconds * 1000);

    for(size_t i=0; i<resources->dbCluster.size(); i++)
    {
        try
        {
            resources->dbCluster[i]->execSql(statement);
        }
        catch(const std::exception &e)
        {
            Wt::log("info") << "error: " << e.what();
        }
    }

    **/
}

void Trokam::SearchWidget::search(const std::string &terms)
{
    // setDbTimeOut(4);
    Wt::log("info") << "+++++++ search(..)";

    std::string lowCaseTerms= Xapian::Unicode::tolower(terms);
    // TODO: Use xapian method to lower case.
    // boost::algorithm::to_lower(lowCaseTerms);

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

    items_found =
        shared_resources->
            readable_content_db.search(
                lowCaseTerms,
                language_selected,
                results_requested);

    // TODO: perhaps the variable total_results is superfluous
    // total_results = items_found.size();
    current_page = 1;
}

    /**
    std::string explanation;
    explanation+= "<p><span class=\"text-success\">";
    explanation+= "total= &alpha;*rb + &beta;*ru + &gamma;*rt<br>";
    explanation+= "where &alpha;= 1.0 &beta;= 17.0 &gamma;= 5.0</span><br/></p>";

    auto oneRow = std::make_unique<Wt::WTemplate>();
    oneRow->setTemplateText(explanation, Wt::TextFormat::UnsafeXHTML);
    userFindings->elementAt(0, 0)->addWidget(std::move(oneRow));
    **/

void Trokam::SearchWidget::show_search_results()
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
            out+= "<p>&nbsp;<br/>";
            // out+= "<span style=\"font-size:x-large;\">" + items_found[i].title + "</span><br/>";
            // out+= "<a href=\"" + items_found[i].url + "\" target=\"_blank\">" + "<span style=\"font-size:x-large;\">" + items_found[i].title + "</span>" + "</a><br/>";
            // out+= "<span class=\"text-success\">" + std::to_string((int)i) + "</span>";
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
            // out+= "&nbsp;<br/>";

            // auto oneRow = std::make_unique<Wt::WTemplate>(out);
            auto oneRow = std::make_unique<Wt::WTemplate>();
            oneRow->setTemplateText(out, Wt::TextFormat::UnsafeXHTML);
            userFindings->elementAt(i, 0)->addWidget(std::move(oneRow));
        }
    }
    else
    {
        layout()->itemAt(SEARCH_STATE)->widget()->setHidden(false);
    }
}
    /**
    const std::string likeClause= Trokam::TextProcessing::generateLikeClause(lowCaseTerms);

    if (likeClause == "")
    {
        return;
    }

    std::string statement;
    statement = "SELECT index_page, url, title, relevance_in_body, relevance_in_url, relevance_in_title, relevance_total, snippet, words.value ";
    statement+= "FROM findings, words ";
    statement+= "WHERE words.index=findings.index_words ";
    statement+= likeClause;
    statement+= "ORDER BY relevance_total ";
    statement+= "DESC LIMIT 20 ";

    Wt::log("info") << "sql sentence: '" << statement << "'";

    Trokam::Bundle<Trokam::Findings> results;

    const int lenSearch= lowCaseTerms.length();

    // Database queries are executed in parallel, each one in a thread.
    // boost::thread_group takes ownership of thread pointers and
    // takes care to delete each one in its destructor.
    boost::thread_group tg;
    for(size_t i=0; i<resources->dbCluster.size(); i++)
    {
        boost::thread *t= new boost::thread(boost::bind(&Trokam::SearchWidget::getFindings, this, statement, i, lenSearch));
        tg.add_thread(t);
    }
    tg.join_all();

    Trokam::Findings item;
    while(bagFindings.pop(item))
    {
        results.insert(item);
    }


    std::sort(results.package.begin(),
              results.package.end(),
              [](Trokam::Findings a, Trokam::Findings b)
                {
                    return (a.relevanceTotal > b.relevanceTotal);
                }
            );


    urlCollection.clear();
    userFindings->clear();

    layout()->itemAt(SEARCH_STATE)->widget()->setHidden(true);

    if(results.size() != 0)
    {
        for(int i= 0; i<results.size(); i++)
        {
            if (urlShown(results[i].url) == false)
            {
                std::string snippet;
                const std::string index= std::to_string(results[i].pageIndex);

                std::string out;
                out+= "<p>";
                out+= "<span style=\"font-size:x-large;\">" + results[i].title + "</span><br/>";
                out+= "<strong><a href=\"" + results[i].url + "\" target=\"_blank\">" + results[i].url + "</a></strong><br/>";
                out+= results[i].snippet + "<br/>";
                out+= "<span class=\"text-success\"> phrase: <strong>" + results[i].phrase
                      + "</strong> / matching: <strong>" + std::to_string(results[i].phraseMatching)
                      + "</strong> / relevance in body: <strong>" + std::to_string(results[i].relevanceInBody)
                      + "</strong> / relevance in URL: <strong>" + std::to_string(results[i].relevanceInUrl)
                      + "</strong> / relevance in title: <strong>" + std::to_string(results[i].relevanceInTitle)
                      + "</strong> / total relevance: <strong>" + std::to_string(results[i].relevanceTotal)
                      + "</strong></span><br/>${button}";
                out+= "</p>";
                out+= "&nbsp;<br/>";

                // auto oneRow = Wt::cpp14::make_unique<Wt::WTemplate>(out);
                auto oneRow = Wt::cpp14::make_unique<Wt::WTemplate>();
                oneRow->setTemplateText(out, Wt::TextFormat::UnsafeXHTML);

                const std::string title= results[i].title;
                const std::string url=   results[i].url;
                const int dbId=          results[i].dbId;

                auto fullInfo = oneRow->bindWidget("button", Wt::cpp14::make_unique<Wt::WPushButton>("page analysis"));
                fullInfo->setStyleClass("btn btn-default btn-xs");
                fullInfo->clicked().connect([=] {
                                    const std::string &terms               showAnalysis(url, title, dbId, index);
                                                });
                fullInfo->setHidden(false);

                userFindings->elementAt(i, 0)->addWidget(std::move(oneRow));
            }
        }
    }
    else
    {
        layout()->itemAt(SEARCH_STATE)->widget()->setHidden(false);
    }
    **/
//}

/**
void Trokam::SearchWidget::getFindings(const std::string &sentence,
                                       const int &dbId,
                                       const int &lenSearch)
{
    Wt::log("info") << "getFindings -- dbId: " << dbId;

    try
    {
        pqxx::result answer;
        resources->dbCluster[dbId]->execSql(sentence, answer);

        for (pqxx::result::iterator row = answer.begin(); row != answer.end(); row++)
        {
            Trokam::Findings item;

            item.pageIndex=        row[0].as(int());
            item.url=              row[1].as(std::string());
            item.title=            row[2].as(std::string());
            item.relevanceInBody=  row[3].as(int());
            item.relevanceInUrl=   row[4].as(int());
            item.relevanceInTitle= row[5].as(int());
            item.relevanceTotal=   row[6].as(int());
            item.snippet=          row[7].as(std::string());
            item.phrase=           row[8].as(std::string());

            const int lenFound= item.phrase.length();
            const float diff= std::abs(lenFound - lenSearch);
            const int matching= std::max(int(100*std::exp(-diff*0.2)),1);

            item.relevanceTotal=   item.relevanceTotal*matching;
            item.phraseMatching=   matching;
            item.dbId=             dbId;

            bagFindings.push(item);
        }
    }
    catch(const std::exception &e)
    {
        Wt::log("info") << "error: " << e.what();
    }
}
**/

bool Trokam::SearchWidget::urlShown(const std::string &url)
{
    for(std::vector<std::string>::iterator it= urlCollection.begin(); it!=urlCollection.end(); ++it)
    {
        const std::string sto= *it;
        if (sto==url)
        {
            return true;
        }
    }

    /**
     * Reaching here means that the url was not found in
     * the collection. A new one is inserted.
     **/
    urlCollection.push_back(url);
    return false;
}

void Trokam::SearchWidget::phrasesPopupSelect(Wt::WMenuItem *item)
{
    Wt::log("info") << __PRETTY_FUNCTION__;

    phraseOnFocus= -1;

    layout()->itemAt(0)->widget()->setHidden(true);   /// General Info
    layout()->itemAt(1)->widget()->setHidden(false);  /// Small logo
    layout()->itemAt(2)->widget()->setHidden(true);   /// Big log
    layout()->itemAt(4)->widget()->setHidden(true);   /// Brief intro

    const std::string choice= item->text().toUTF8();
    userInput->setText(choice);
    userInput->setFocus(true);

    application->processEvents();

    search(choice);
    show_search_results();
    generateFooter();
}

void Trokam::SearchWidget::showAnalysis(const std::string &url,
                                        const std::string &title,
                                        const int &dbId,
                                        const std::string &urlIndex)
{
    /**

    Wt::log("info") << __PRETTY_FUNCTION__;
    Wt::log("info") << "url: " << url << " dbId: " << dbId << " urlIndex: " << urlIndex;

    setDbTimeOut(10);

    auto pageInfo = std::make_unique<Wt::WTable>();

    const Wt::WEnvironment& env = Wt::WApplication::instance()->environment();
    if(env.agentIsMobileWebKit())
    {
        try
        {
            std::string statement;
            statement = "SELECT value, relevance_in_body, relevance_in_url, relevance_in_title, relevance_total, crunched ";
            statement+= "FROM words, findings ";
            statement+= "WHERE findings.index_words=words.index ";
            statement+= "AND index_page=" + urlIndex + " ";
            statement+= "ORDER BY crunched DESC, relevance_total DESC ";
            statement+= "LIMIT 7 ";

            // Wt::log("info") << "statement: " << statement;

            pageInfo->addStyleClass("table table-condensed table-striped");

            pqxx::result answer;
            resources->dbCluster[dbId]->execSql(statement, answer);

            pageInfo->elementAt(0, 0)->addWidget(std::make_unique<Wt::WText>("<strong>phrase</strong>"));

            int k= 1;
            for (pqxx::result::iterator row = answer.begin(); row != answer.end(); row++)
            {
                pageInfo->elementAt(k, 0)->addWidget(std::make_unique<Wt::WText>(row[0].as(std::string())));
                k++;
            }
        }
        catch(const std::exception &e)
        {
            Wt::log("info") << "error: " << e.what();
        }
    }
    else
    {
        try
        {
            std::string statement;
            statement = "SELECT value, relevance_in_body, relevance_in_url, relevance_in_title, relevance_total, crunched ";
            statement+= "FROM words, findings ";
            statement+= "WHERE findings.index_words=words.index ";
            statement+= "AND index_page=" + urlIndex + " ";
            statement+= "ORDER BY crunched DESC, relevance_total DESC ";
            statement+= "LIMIT 10 ";

            // Wt::log("info") << "statement: " << statement;

            pageInfo->addStyleClass("table table-striped");

            pqxx::result answer;
            resources->dbCluster[dbId]->execSql(statement, answer);

            pageInfo->elementAt(0, 0)->addWidget(std::make_unique<Wt::WText>("<strong>phrase</strong>"));
            pageInfo->elementAt(0, 1)->addWidget(std::make_unique<Wt::WText>("<strong>relev. in body</strong>"));
            pageInfo->elementAt(0, 2)->addWidget(std::make_unique<Wt::WText>("<strong>relev. in url</strong>"));
            pageInfo->elementAt(0, 3)->addWidget(std::make_unique<Wt::WText>("<strong>relev. in title</strong>"));

            int k= 1;
            for (pqxx::result::iterator row = answer.begin(); row != answer.end(); row++)
            {
                pageInfo->elementAt(k, 0)->addWidget(std::make_unique<Wt::WText>(row[0].as(std::string())));
                pageInfo->elementAt(k, 1)->addWidget(std::make_unique<Wt::WText>(row[1].as(std::string())));
                pageInfo->elementAt(k, 2)->addWidget(std::make_unique<Wt::WText>(row[2].as(std::string())));
                pageInfo->elementAt(k, 3)->addWidget(std::make_unique<Wt::WText>(row[3].as(std::string())));
                k++;
            }
        }
        catch(const std::exception &e)
        {
            Wt::log("info") << "error: " << e.what();
        }
    }

    auto header = std::make_unique<Wt::WText>(Wt::WString("URL: ") + url + Wt::WString("<br/>Title: ") + title);

    auto language_box = addChild(std::make_unique<Wt::WDialog>("Most relevant phrases"));

    auto closeButton = std::make_unique<Wt::WPushButton>("Close");
    closeButton->addStyleClass("btn btn-primary");
    closeButton->clicked().connect([=] {
                                            removeChild(language_box);
                                       });

    // Process the dialog result.
    language_box->finished().connect([=] {
                                            removeChild(language_box);
                                        });

    language_box->titleBar()->addWidget(std::move(header));
    language_box->contents()->addWidget(std::move(pageInfo));
    language_box->footer()->addWidget(std::move(closeButton));
    language_box->rejectWhenEscapePressed();
    language_box->setModal(false);
    language_box->show();

    **/
}

void Trokam::SearchWidget::showLanguageOptions()
{
    auto preferences_box = addChild(std::make_unique<Wt::WDialog>("Preferences"));
    // auto header = std::make_unique<Wt::WText>(Wt::WString("Search Languages"));


    auto language_choices = std::make_unique<Wt::WTable>();
    language_choices->addStyleClass("w3-table");

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

bool Trokam::SearchWidget::savePreferences()
{
    std::string language_selected;
    for(unsigned int i=0; i<language_options.size(); i++)
    {
        if(std::get<bool>(language_options[i]))
        {
            language_selected += std::to_string(i) + ",";
        }
    }

    application->setCookie("languages", language_selected, 10000000);

    return true;
}

void Trokam::SearchWidget::generateFooter()
{
    Wt::log("info") << "+++++++ generateFooter() +++ 0";

    if(!ptr_pagination)
    {
        Wt::log("info") << "ptr_pagination is null";
        return;
    }

    int total_results = items_found.size();

    Wt::log("info") << "+++++++ generateFooter() +++ 1";
    Wt::log("info") << "total_results:" << total_results;
    Wt::log("info") << "results_per_page:" << results_per_page;

    ptr_pagination->clear();
    ptr_pagination->setTemplateText("");

    if(total_results==0)
    {
        Wt::log("info") << "+++++++ generateFooter() +++ 1.1";
        return;
    }

    auto dv = std::div(total_results, results_per_page);

    int total_pages = dv.quot;
    if(dv.rem > 0)
    {
        total_pages++;
    }

    Wt::log("info") << "dv.quot:" << dv.quot;
    Wt::log("info") << "dv.rem:" << dv.rem;

    // Template for pagination of search results
    std::string tpl_pagination =
        "<nav aria-label=\"Page navigation\">\n"
        "<ul class=\"pagination\">\n";

    if(current_page == 1)
    {
        tpl_pagination+= "    <li class=\"disabled\">${link_previous}</li>\n";
    }
    else
    {
        tpl_pagination+= "    <li>${link_previous}</li>\n";
    }

    // The page numbering start from one.
    for(int i=1; i<=total_pages; i++)
    {
        if(current_page == i)
        {
            tpl_pagination += "    <li class=\"active\">${link_group_" + std::to_string(i) + "}</li>\n";
        }
        else
        {
            tpl_pagination += "    <li>${link_group_" + std::to_string(i) + "}</li>\n";
        }
    }

    if(current_page == total_pages)
    {
        tpl_pagination+= "    <li class=\"disabled\">${link_next}</li>\n";
    }
    else
    {
        tpl_pagination+= "    <li>${link_next}</li>\n";
    }

    tpl_pagination +=
        "</ul>\n"
        "</nav>\n";

    Wt::log("info") << "tpl_pagination:\n" << tpl_pagination;

    // auto wt_pagination = std::make_unique<Wt::WTemplate>(tpl_pagination);
    ptr_pagination->setTemplateText(tpl_pagination, Wt::TextFormat::UnsafeXHTML);

    Wt::log("info") << "+++++++ generateFooter() +++ 2";

    Wt::WString inner_previous = "<span aria-hidden=\"true\"><span aria-hidden=\"true\">&larr;</span> previous</span>";
    auto wt_previous = std::make_unique<Wt::WAnchor>(Wt::WLink("#"), inner_previous);
    wt_previous->clicked().connect(
        [=] {
            Wt::log("info") << "previous";
            if(current_page > 1)
            {
                current_page--;
                show_search_results();
                generateFooter();
            }
        });
    ptr_pagination->bindWidget("link_previous", std::move(wt_previous));

    Wt::WString inner_next = "<span aria-hidden=\"true\">next <span aria-hidden=\"true\">&rarr;</span></span>";
    auto wt_next = std::make_unique<Wt::WAnchor>(Wt::WLink("#"), inner_next);
    wt_next->clicked().connect(
        [=] {
            Wt::log("info") << "next";
            if(current_page < total_pages)
            {
                current_page++;
                show_search_results();
                generateFooter();
            }
        });
    ptr_pagination->bindWidget("link_next", std::move(wt_next));

    for(int i=1; i<=total_pages; i++)
    {
        // The page numbering start from one.
        auto wt_group = std::make_unique<Wt::WAnchor>(Wt::WLink("#"), std::to_string(i));
        // wt_group->clicked().connect([=] { Wt::log("info") << "show page" << std::to_string(i+1); });
        wt_group->clicked().connect(
            [=] {
                current_page = i;
                show_search_results();
                generateFooter();
            });

        std::string group_id = "link_group_" + std::to_string(i);
        ptr_pagination->bindWidget(group_id, std::move(wt_group));
    }

    application->processEvents();

    Wt::log("info") << "+++++++ generateFooter() +++ 3";
}