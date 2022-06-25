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

// Boost
#include <boost/algorithm/string.hpp>
#include <boost/thread.hpp>

// Wt
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
#include <Wt/WTemplate.h>

// Trokam
#include "deferredWidget.h"
#include "searchWidget.h"

/// Trokam
#include "bundle.h"
#include "common.h"
#include "file_ops.h"


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
     * Information about the results.
     **/
    auto infoResults =
        std::make_unique<Wt::WTemplate>(
            Wt::WString::tr("info-results"));
    vbox->addWidget(std::move(infoResults));

    /**
     * The box on which the results are displayed.
     **/
    auto subStack = std::make_unique<Wt::WStackedWidget>();
    subStack->addStyleClass("contents");
    subStack->setOverflow(Wt::Overflow::Auto);

    auto findingsBox =
        std::make_unique<Wt::WTemplate>(
            Wt::WString::tr("findings-box"));
    userFindings =
        findingsBox->bindWidget(
            "user-findings", std::make_unique<Wt::WTable>());
    subStack->addWidget(std::move(findingsBox));
    vbox->addWidget(std::move(subStack), 1);

    vbox->itemAt(GENERAL_INFO)->widget()->setHidden(false);
    vbox->itemAt(SMALL_LOGO)->widget()->setHidden(true);
    vbox->itemAt(BIG_LOGO)->widget()->setHidden(false);
    vbox->itemAt(BRIEF_INTRO)->widget()->setHidden(false);
    vbox->itemAt(SEARCH_STATE)->widget()->setHidden(true);

    setLayout(std::move(vbox));

    timer = app->root()->addChild(std::make_unique<Wt::WTimer>());
    timer->setInterval(std::chrono::milliseconds(750));
    timer->timeout().connect(this, &Trokam::SearchWidget::timeout);

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
    }

    if ((kEvent.key() == Wt::Key::Down) && (phrasesPopup))
    {
        userInput->setFocus(false);
        std::vector<Wt::WMenuItem*> alternatives= phrasesPopup->items();
        alternatives[0]->setFocus(true);
        phraseOnFocus= 0;
    }
}

void Trokam::SearchWidget::phrasesPopupKeyPressed(const Wt::WKeyEvent &kEvent)
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
    // searchForPhrases();
}

void Trokam::SearchWidget::textInput()
{
    Wt::log("info") << __PRETTY_FUNCTION__;
    timer->start();
}

/**
void Trokam::SearchWidget::searchForPhrases()
{
    Wt::log("info") << __PRETTY_FUNCTION__;

    std::string text= userInput->text().toUTF8();
    boost::algorithm::to_lower(text);

    setDbTimeOut(1);

    if(text.length() >= 2)
    {
        sequenceCollection.clear();

        const std::string likeClause= Trokam::TextProcessing::generateLikeClause(text);

        if (likeClause == "")
        {
            return;
        }

        std::string sentence;
        sentence=  "SELECT index, value, count ";
        sentence+= "FROM words ";
        sentence+= "WHERE count>0 ";
        sentence+= likeClause;
        sentence+= "ORDER BY count DESC ";
        sentence+= "LIMIT 10 ";

        Wt::log("info") << "sql sentence: '" << sentence << "'";


        // Database queries are executed in parallel, each one in a thread.
        // boost::thread_group takes ownership of thread pointers and
        // takes care to delete each one in its destructor.

        boost::thread_group tg;
        for(size_t i=0; i<resources->dbCluster.size(); i++)
        {
            boost::thread *t= new boost::thread(boost::bind(&Trokam::SearchWidget::getPhrases, this, sentence, i));
            tg.add_thread(t);
        }
        tg.join_all();

        Trokam::Sequence seq;
        while(bagPhrases.pop(seq))
        {
            insertSequence(seq);
        }

        std::sort(sequenceCollection.begin(),
                  sequenceCollection.end(),
                  [](Trokam::Sequence a, Trokam::Sequence b)
                    {
                        return (a.occurrences > b.occurrences);
                    }
                );

        if(phrasesPopup)
        {
            delete phrasesPopup.release();
        }

        phrasesPopup = std::make_unique<Wt::WPopupMenu>();
        phrasesPopup->itemSelected().connect(this, &Trokam::SearchWidget::phrasesPopupSelect);

        for(size_t i= 0; ((i<sequenceCollection.size()) && (i<15)); i++)
        {
            phrasesPopup->addItem(sequenceCollection[i].value);
        }

        std::vector<Wt::WMenuItem*> alternatives= phrasesPopup->items();
        for(unsigned int i=0; i<alternatives.size(); i++)
        {
            alternatives[i]->setCanReceiveFocus(true);
            alternatives[i]->keyWentDown().connect(this, &SearchWidget::phrasesPopupKeyPressed);
        }

        phrasesPopup->popup(userInput);

        if(sequenceCollection.size() > 0)
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
**/

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

    std::string lowCaseTerms= terms;
    boost::algorithm::to_lower(lowCaseTerms);

    std::string languages = "english";
    unsigned int offset = 1;
    unsigned int page_size = 15;

    std::vector<Trokam::Finding> items_found =
        shared_resources->
            readable_content_db.search(
                lowCaseTerms,
                languages,
                offset,
                page_size);


    if(items_found.size() != 0)
    {
        for(size_t i= 0; i<items_found.size(); i++)
        {
            std::string out;
            out+= "<p>";
            out+= "<span style=\"font-size:x-large;\">" + items_found[i].title + "</span><br/>";
            out+= "<strong><a href=\"" + items_found[i].url + "\" target=\"_blank\">" + items_found[i].url + "</a></strong><br/>";
            out+= items_found[i].snippet + "<br/>";
            out+= "</p>";
            out+= "&nbsp;<br/>";

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
                                                    showAnalysis(url, title, dbId, index);
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
}

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

    auto analysisBox = addChild(std::make_unique<Wt::WDialog>("Most relevant phrases"));

    auto closeButton = std::make_unique<Wt::WPushButton>("Close");
    closeButton->addStyleClass("btn btn-primary");
    closeButton->clicked().connect([=] {
                                            removeChild(analysisBox);
                                       });

    // Process the dialog result.
    analysisBox->finished().connect([=] {
                                            removeChild(analysisBox);
                                        });

    analysisBox->titleBar()->addWidget(std::move(header));
    analysisBox->contents()->addWidget(std::move(pageInfo));
    analysisBox->footer()->addWidget(std::move(closeButton));
    analysisBox->rejectWhenEscapePressed();
    analysisBox->setModal(false);
    analysisBox->show();

    **/
}
