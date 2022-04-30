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
#include <cstdlib>
#include <iomanip>
#include <iostream>

// Trokam
#include "grasp.h"
#include "file_ops.h"
#include "plain_text_processor.h"

Trokam::Grasp::Grasp()
{
    // std::string dbpath;

    if(const char* env_home = std::getenv("HOME"))
    {
        // std::cout << "Your PATH is: " << env_p << '\n';
        db_path  = env_home;
        db_path += "/grasp";
    }
    else
    {
        std::cerr << "fail: could not create grasp database.";
        exit(1);
    }

    db.reset(
        new Xapian::WritableDatabase(
            db_path, Xapian::DB_CREATE_OR_OPEN));
}

void Trokam::Grasp::insert(
    const int &id,
    const std::string &url,
    const std::string &title,
    const std::string &text)
{
    // We make a document and tell the term generator to use this.
    Xapian::Document doc;
    Xapian::TermGenerator term_generator;    
    term_generator.set_document(doc);

    // Index fields without prefixes for general search.
    term_generator.index_text(text);

    // We use the identifier to ensure each object ends up in the
    // database only once no matter how many times we run the
    // indexer.
    std::string id_term = "Q" + std::to_string(id);
    std::cout  << "indexing page with id:" << id_term << "\n";

    // doc.set_data(title);
    // doc.set_data(std::to_string(id) + " -- " + title);
    doc.set_data(text);
    doc.add_value(SLOT_URL, url);
    doc.add_value(SLOT_TITLE, title);
    doc.add_boolean_term(id_term);
    db->replace_document(id_term, doc);
}

void Trokam::Grasp::search(
    const std::string &querystring,
    Xapian::doccount offset,
    Xapian::doccount pagesize)
{
    // Set up a QueryParser with a stemmer and suitable prefixes.
    Xapian::QueryParser queryparser;
    // -- queryparser.set_stemmer(Xapian::Stem("en"));
    queryparser.set_stemming_strategy(queryparser.STEM_SOME);

    // And parse the query.
    Xapian::Query query = queryparser.parse_query(querystring);

    // Use an Enquire object on the database to run the query.
    Xapian::Enquire enquire(*db);
    enquire.set_query(query);

    // And print out something about each match.
    Xapian::MSet mset = enquire.get_mset(offset, pagesize);

    for(Xapian::MSetIterator m = mset.begin(); m != mset.end(); ++m)
    {
        // std::cout << "description:" << m.get_description() << "\n\n";
        std::cout << m.get_document().get_value(SLOT_TITLE) << '\n';
        std::cout << "rank:" << m.get_rank() << " -- weight:" << m.get_weight () << "\n";
        std::cout << m.get_document().get_value(SLOT_URL) << '\n';

        const std::string &data = m.get_document().get_data();
        const std::string snippet =
            Trokam::PlainTextProcessor::snippet(data, querystring, 250);
        std::cout << snippet << "\n\n";

        std::cout << '\n';
    }
    std::cout << '\n';
}

void Trokam::Grasp::clean()
{
    Trokam::FileOps::rmDir(db_path);
}
