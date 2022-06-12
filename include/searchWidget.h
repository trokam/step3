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

#ifndef TROKAM_SEARCH_WIDGET_H
#define TROKAM_SEARCH_WIDGET_H

/// C++
#include <memory>

/// Boost
#include <boost/lockfree/stack.hpp>
#include <boost/shared_ptr.hpp>

/// Wt
#include <Wt/WEvent.h>
#include <Wt/WLineEdit.h>
#include <Wt/WPopupMenu.h>
#include <Wt/WTable.h>
#include <Wt/WText.h>
#include <Wt/WTimer.h>

/// Trokam
#include "pageWidget.h"
#include "bundle.h"
#include "data.h"
#include "sharedResources.h"

namespace Trokam
{
    class SearchWidget : public Trokam::PageWidget
    {
        public:

            SearchWidget(boost::shared_ptr<Trokam::SharedResources> &sr, Wt::WApplication* app);
            ~SearchWidget();
            void populateSubMenu(Wt::WMenu *menu);

        private:

            int phraseOnFocus;

            std::unique_ptr<Wt::WPopupMenu> phrasesPopup;
            std::unique_ptr<Wt::WText>      searchState;

            Wt::WLineEdit *userInput;
            Wt::WTable    *userFindings;
            Wt::WTimer    *timer;

            std::vector<Trokam::Sequence> sequenceCollection;
            boost::lockfree::stack<Trokam::Sequence> bagPhrases{100};
            boost::lockfree::stack<Trokam::Findings> bagFindings{100};
            std::vector<std::string> urlCollection;

            void keyPressedEntrance(const Wt::WKeyEvent &kEvent);
            void phrasesPopupKeyPressed(const Wt::WKeyEvent &kEvent);
            void phrasesPopupSelect(Wt::WMenuItem *item);

            void timeout();
            void textInput();
            // void searchForPhrases();
            void getPhrases(const std::string &sentence,
                            const int &dbId);
            void insertSequence(const Trokam::Sequence &seq);
            void setDbTimeOut(const int &timeOutSeconds);
            void search(const std::string &terms);
            void getFindings(const std::string &sentence,
                             const int &dbId,
                             const int &lenSearch);
            bool urlShown(const std::string &url);

            void showAnalysis(const std::string &url,
                              const std::string &title,
                              const int &dbId,
                              const std::string &urlIndex);
    };
}

#endif /// TROKAM_SEARCH_WIDGET_H
