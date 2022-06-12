/***********************************************************************
 *                            T R O K A M
 *                       Internet Search Engine
 *
 * Copyright (C) 2018, Nicolas Slusarenko
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

#ifndef TROKAM_TEXTSTORE_H
#define TROKAM_TEXTSTORE_H

/// C++
#include <string>
#include <vector>

namespace Trokam
{
    struct TextOcc
    {
        std::string text;
        int index;
        int occurrence;
        int relevanceInBody;
        int relevanceInUrl;
        int relevanceInTitle;
        int relevanceTotal;
    };

    class TextStore
    {
        public:

            TextStore();
            void insert(const std::string &text);

            /**
             * Show the first sequences,
             * sorted descendently by occurrence.
             **/
            void show(const int &value= -1);

            void setRelevance(const int &total,
                              const std::string &title,
                              const std::string &url);
            int size() const;

            TextOcc get(const int &id) const;

            TextOcc& operator[](const int &index)
            {
                return textCollection.at(index);
            };


        private:

            bool sorted;
            std::vector<TextOcc> textCollection;

            void sortRelevanceBody();
            void sortRelevanceTotal();
    };
}

#endif  /// TROKAM_TEXTSTORE_H
