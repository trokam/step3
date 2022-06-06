/***********************************************************************
 *                            T R O K A M
 *                       Internet Search Engine
 *
 * Copyright (C) 2022, Nicolas Slusarenko
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

#pragma once

extern "C" {
#include "textcat/textcat.h"
}

namespace Trokam
{
    class LanguageDetection
    {
        public:

            LanguageDetection()
            {
                lib_handle = textcat_Init(config_file.c_str());
            }

            ~LanguageDetection()
            {
                if(lib_handle != nullptr)
                {
                    textcat_Done(lib_handle);
                }
            }

            std::string detectLanguage(
                const std::string &content)
            {
                if(content.length() < 10)
                {
                    return "unknown";
                }

                char *language =
                    textcat_Classify(
                        lib_handle,
                        content.c_str(),
                        content.length());

                std::string result = language;
                if((result == "SHORT") || (result == "UNKNOWN"))
                {
                    return "unknown";
                }
 
                size_t second_braket = result.find(']', 1);
                result = result.substr(1, second_braket-1);
                return result;                
            }

        private:

            const std::string config_file = "/usr/local/etc/trokam/language.conf";
            void *lib_handle = nullptr;
    };
}
