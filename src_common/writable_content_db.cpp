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
#include <cstdlib>
#include <iomanip>
#include <iostream>

// Trokam
#include "writable_content_db.h"
#include "file_ops.h"
#include "plain_text_processor.h"

Trokam::WritableContentDB::WritableContentDB(
    Trokam::Options &opt)
    : options(opt)
{
    db_path = options.dbContent();

    db.reset(
        new Xapian::WritableDatabase(
            db_path, Xapian::DB_CREATE_OR_OPEN));
}

/**
 * Insert the document into de page-database.
 **/
void Trokam::WritableContentDB::insert(
    const int &id,
    const std::string &url,
    const std::string &title,
    const std::string &text,
    const std::string &language)
{
    // Make a document and tell the term generator to use this.
    Xapian::Document doc;
    Xapian::TermGenerator term_generator;
    term_generator.set_document(doc);

    // Index the text.
    term_generator.index_text(text);

    // Use an identifier to ensure each object ends up in the
    // database only once. This identifier is the same one
    // as the one used in the link-database.
    const std::string id_term = "Q" + std::to_string(id);
    std::cout  << "indexing page with id:" << id_term << "\n";

    // Set the language.
    const std::string lang_term = "L" + language;

    doc.set_data(text);
    doc.add_value(SLOT_URL, url);
    doc.add_value(SLOT_TITLE, title);
    doc.add_boolean_term(id_term);
    doc.add_boolean_term(lang_term);
    db->replace_document(id_term, doc);
}

/**
 * This method deletes everything under db_path.
 **/
void Trokam::WritableContentDB::clean()
{
    Trokam::FileOps::cleanDir(db_path);
}
