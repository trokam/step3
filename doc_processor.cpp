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

// C++
#include "iostream"

// Trokam
#include "doc_processor.h"
#include "plain_text_processor.h"
#include "language_detection.h"

void Trokam::DocProcessor::show(
    const web_doc *doc,
    const std::string &retrieval_error,
    const int &set_for_download)
{
    title.clear();
    text.clear();
    lang.clear();

    const std::string &raw = doc->raw;

    std::cout << "\n\n================================== " << set_for_download
                << " ==================================\n";
    std::cout << "indexed URL:" << doc->url << '\n';
    std::cout << "doc_id:" << doc->id << '\n';
    std::cout << "content_type:" << doc->content_type << '\n';
    std::cout << "error code:" << retrieval_error << '\n';
    std::cout << "page length:" << doc->raw.length() << '\n';

    title = Trokam::PlainTextProcessor::getTitle(raw);
    Trokam::PlainTextProcessor::extractPlainText(
        TEXT_LENGTH_LIMIT, raw, text);

    std::cout << "page title:" << title << '\n';        
    std::cout << "page content:" << text.substr(0, 300) << '\n';

    Trokam::LanguageDetection ld;
    lang = ld.detectLanguage(text);
    std::cout << "page lang:" << lang << '\n';            
}
