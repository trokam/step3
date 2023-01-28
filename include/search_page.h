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

#pragma once

// C++
#include <memory>
#include <string>

// Wt
#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WNavigationBar.h>
#include <Wt/WMessageBox.h>
#include <Wt/WMenu.h>
#include <Wt/WPopupMenu.h>
#include <Wt/WSelectionBox.h>
#include <Wt/WStringListModel.h>
#include <Wt/WSuggestionPopup.h>
#include <Wt/WTimer.h>

// Trokam
#include "preferences.h"
#include "shared_resources.h"

namespace Trokam
{
    class SearchPage : public Wt::WContainerWidget
    {
        public:

            SearchPage(
                boost::shared_ptr<Trokam::SharedResources> &sr,
                Wt::WApplication* app);

        private:

            int results_per_page = 8;
            int current_page = 1;

            Trokam::Preferences user_settings;

            Wt::WApplication *application;
            Wt::WContainerWidget *container = nullptr;
            Wt::WLineEdit *input = nullptr;
            Wt::WTable *userFindings = nullptr;
            Wt::WContainerWidget *w_footer = nullptr;
            Wt::WPushButton *w_button_preferences = nullptr;
            Wt::WPushButton *w_button_sponsors = nullptr;
            Wt::WPushButton *w_about = nullptr;
            Wt::WSelectionBox *w_sugggestion_box = nullptr;
            Wt::WTimer *timer = nullptr;
            std::shared_ptr<Wt::WButtonGroup> group;

            boost::shared_ptr<Trokam::SharedResources> shared_resources;
            std::vector<Trokam::Finding> items_found;
            std::vector<std::pair<int, bool>> language_options;

            void search(const std::string &terms);
            void showSearchResults();
            void handlePathChange();
            void createFooter(Wt::WContainerWidget *base);
            void showUserOptions();
            bool savePreferences();
            void inputKeyWentUp(const Wt::WKeyEvent &kEvent);
            void suggestionBoxKeyWentUp(const Wt::WKeyEvent &kEvent);
            void suggestionBoxEnterPressed();
            void timeout();
            void textInput();
            void showSuggestions();
            void createSuggestionBox();
            void destroySuggestionBox();
            bool isAgentMobile();
            void checkTraining();
            void training();

            /**
             * Generates a human readable string with the languages
             * selected to perform the search.
             * For example: Spanish, Vietnamese, Polish and Indonesian.
             **/
            std::string getLanguagesSelected();
    };
}
