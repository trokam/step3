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

// C++
#include "iostream"

// Boost
#include <boost/algorithm/string.hpp>

// Trokam
#include "doc_processor.h"
#include "file_ops.h"
#include "plain_text_processor.h"

Trokam::DocProcessor::DocProcessor(
    Trokam::Options &opt)
    : options(opt), writable_content_db(opt)
{}

void Trokam::DocProcessor::insert(
    const web_doc *doc,
    const std::string &retrieval_error,
    const int &set_for_download)
{
    // Save the file as it was downloaded, i.e. in HTML format.
    const std::string &raw = doc->raw;
    Trokam::FileOps::save("/tmp/trokam_raw", raw);

    // Extract the plain text out of HTML content.
    const int status = extractPlainText();

    // Extract the title of the HTML content.
    extractTitle();

    // Identifythe language of the text.
    const std::string language = ld.detectLanguage(text);

    // Display data of the document.
    show(doc, set_for_download, status, retrieval_error, title, language);

    // Insert a new document into word's database only if it's
    // language is well identified.
    if(language != "unknown")
    {
        writable_content_db.insert(doc->id, doc->url, title, text, language);
    }
}

int Trokam::DocProcessor::extractPlainText()
{
    // This convertion uses a modified version of lynx.
    // This command assumes that the HTML file was saved
    // in the file '/tmp/trokam_raw'.
    std::string command =
        "lynx_mod "
        "-dump -force_html -nolist "
        "-cfg=/usr/local/etc/lynx_mod.cfg "
        "-assume_charset=utf-8 "
        "-assume_local_charset=utf-8 "
        "-display_charset=utf-8 "
        "/tmp/trokam_raw > /tmp/trokam_content";
    const int result = system(command.c_str());

    // Clean the text.
    // lynx generates a text representation for user input fields
    // and submit buttons. Useful for interaction with a user.
    // This is not such case.
    text = Trokam::FileOps::readLines(5000, "/tmp/trokam_content");
    boost::replace_all(text, "____________________", " ");
    boost::replace_all(text, "_______________", " ");
    boost::replace_all(text, "____", " ");
    boost::replace_all(text, "(BUTTON)", "");

    return result;
}

void Trokam::DocProcessor::extractTitle()
{
    // This assumes that the HTML to plain text was
    // executed using the modified version of lynx.
    // lynx_mod saves the title in the file '/tmp/trokam_title'
    title = Trokam::FileOps::readLines(1, "/tmp/trokam_title");

    // Clean the text.
    // Removing unwanted characters for neat displaying.
    boost::algorithm::trim_if(
        title, boost::algorithm::is_any_of(" \n\r\t\\\""));
    boost::replace_all(title, "   ", " ");
    boost::replace_all(title, "  ", " ");
}

void Trokam::DocProcessor::show(
    const web_doc *doc,
    const int &correlative,
    const int &status,
    const std::string &retrieval_error,
    const std::string &title,
    const std::string &language)
{
    std::cout << "\n\n================================== " << correlative
                << " ==================================\n";
    std::cout << "indexed URL:" << doc->url << '\n';
    std::cout << "doc_id:" << doc->id << '\n';
    std::cout << "content_type:" << doc->content_type << '\n';
    std::cout << "error code:" << retrieval_error << '\n';
    std::cout << "page length:" << doc->raw.length() << '\n';
    std::cout << "page title:" << title << '\n';
    std::cout << "page lang:" << language << '\n';
    std::cout << "convert status:" << status << '\n';
}