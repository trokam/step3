/***********************************************************************
 *                            T R O K A M
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

/// C++
#include <memory>
#include <string>

/// Wt
#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WNavigationBar.h>
#include <Wt/WMessageBox.h>
#include <Wt/WMenu.h>
#include <Wt/WPopupMenu.h>
#include <Wt/WSuggestionPopup.h>

/// Trokam
#include "bundle.h"
#include "data.h"
#include "pageWidget.h"
#include "sharedResources.h"

namespace Trokam
{
    class SearchPage : public Wt::WContainerWidget
    {
        public:

            SearchPage(
                boost::shared_ptr<Trokam::SharedResources> &sr,
                Wt::WApplication* app);

        private:

            Wt::WApplication *application;
            boost::shared_ptr<Trokam::SharedResources> shared_resources;
            int results_per_page = 8;
            int current_page = 1;
            std::vector<Trokam::Finding> items_found;
            std::vector<std::pair<int, bool>> language_options;

            // std::unique_ptr<Wt::WPopupMenu> phrasesPopup;
            Wt::WSuggestionPopup *suggestions = nullptr;

            Wt::WLineEdit *input = nullptr;
            Wt::WTable *userFindings = nullptr;

            void search(const std::string &terms);
            void show_search_results();
            void handlePathChange();

            void createFooter(
                Wt::WContainerWidget *base);

            void suggestionSelected(
                const int index,
                Wt::WFormWidget* widget);

            void keyPressedInput(
                const Wt::WKeyEvent &kEvent);

            int phraseOnFocus = 0;

            void phrasesPopupKeyPressed(
                const Wt::WKeyEvent &kEvent);

            void phrasesPopupSelect(
                Wt::WMenuItem *item);
    };
}
