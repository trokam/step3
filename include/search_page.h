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
#include <thread>

// Libcurl
#include <curl/curl.h>

// Json
#include <nlohmann/json.hpp>

// Wt
#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WNavigationBar.h>
#include <Wt/WMessageBox.h>
#include <Wt/WMenu.h>
#include <Wt/WLabel.h>
#include <Wt/WPopupMenu.h>
#include <Wt/WProgressBar.h>
#include <Wt/WSelectionBox.h>
#include <Wt/WSignal.h>
#include <Wt/WStringListModel.h>
#include <Wt/WSuggestionPopup.h>
#include <Wt/WTemplate.h>
#include <Wt/WText.h>
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
                boost::shared_ptr<Trokam::SharedResources> &sr);

            ~SearchPage();

        private:

            const int results_per_page = 8;
            size_t text_clipping = 400;
            int current_page = 1;
            bool searching = false;
            bool waiting_for_answer = false;
            bool showing_complete_answer = true;
            std::string answer;
            std::string auth_token;

            Trokam::Preferences user_settings;

            CURL *curl = NULL;

            Wt::WApplication *application;
            Wt::WContainerWidget *container = nullptr;
            Wt::WLineEdit *input = nullptr;
            Wt::WTemplate *w_answer = nullptr;
            Wt::WTable *userFindings = nullptr;
            Wt::WContainerWidget *w_footer = nullptr;
            Wt::WProgressBar *w_progress_bar = nullptr;
            Wt::WPushButton *w_button_preferences = nullptr;
            Wt::WPushButton *w_button_sponsors = nullptr;
            Wt::WPushButton *w_contrib = nullptr;
            Wt::WPushButton *w_info = nullptr;
            Wt::WPushButton *w_button_clipping = nullptr;
            Wt::WSelectionBox *w_sugggestion_box = nullptr;
            Wt::WTimer *timer = nullptr;
            std::shared_ptr<Wt::WButtonGroup> group;
            std::thread ai_task;

            boost::shared_ptr<Trokam::SharedResources> shared_resources;
            std::vector<Trokam::Finding> items_found;
            std::vector<std::pair<int, bool>> language_options;

            void search(const std::string &terms);
            void getAiAnswer(Wt::WApplication *app, const std::string &terms);
            void waitingTick(Wt::WApplication *app);
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
            void showAnswer();
            std::string exec_post(const std::string &terms);

            /**
             * Generates a human readable string with the languages
             * selected to perform the search.
             * For example: Spanish, Vietnamese, Polish and Indonesian.
             **/
            std::string getLanguagesSelected();
    };
}
