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
    const std::string &text,
    const std::string &language)
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
    const std::string id_term = "Q" + std::to_string(id);
    std::cout  << "indexing page with id:" << id_term << "\n";

    const std::string lang_term = "L" + language;

    // doc.set_data(title);
    // doc.set_data(std::to_string(id) + " -- " + title);
    doc.set_data(text);
    doc.add_value(SLOT_URL, url);
    doc.add_value(SLOT_TITLE, title);
    doc.add_boolean_term(id_term);
    doc.add_boolean_term(lang_term);    
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

    // --------------------------------------------------

    std::vector<std::string> lang_enquiry;
    lang_enquiry.push_back("english");
    // lang_enquiry.push_back("german");

        // Build a query for each material value
        std::vector<Xapian::Query> lang_queries;
        for(const auto &lang: lang_enquiry)
        {
            std::string lang_term = "L";
            lang_term += lang;
            // material += Xapian::Unicode::tolower(lang);
            lang_queries.push_back(Xapian::Query(lang_term));
        }

        // Combine these queries with an OR operator
        Xapian::Query language_query(
            Xapian::Query::OP_OR,
            lang_queries.begin(),
            lang_queries.end());

        // Use the material query to filter the main query
        query = Xapian::Query(
            Xapian::Query::OP_FILTER,
            query,
            language_query);

    // --------------------------------------------------

    // Use an Enquire object on the database to run the query.
    Xapian::Enquire enquire(*db);
    enquire.set_query(query);

    Xapian::MSet mset = enquire.get_mset(offset, pagesize);

    std::vector<Trokam::DocData> search_results;

    for(Xapian::MSetIterator m = mset.begin(); m != mset.end(); ++m)
    {
        std::string title = m.get_document().get_value(SLOT_TITLE);
        std::string url = m.get_document().get_value(SLOT_URL);

        float title_weight =
            Trokam::PlainTextProcessor::howMuchOf(title, querystring) + 1.0;

        float url_weight =
            Trokam::PlainTextProcessor::howMuchOf(url, querystring) + 1.0;

        float relevance = m.get_weight() * title_weight * title_weight * url_weight * url_weight;
        // m.get_document().add_value(SLOT_RELEVANCE, std::to_string(relevance));

        Trokam::DocData doc;
        doc.it = m;
        doc.relevance = relevance;
        search_results.push_back(doc);
    }
   
    std::sort(
        search_results.begin(), 
        search_results.end(), 
        [](Trokam::DocData a, Trokam::DocData b) {return a.relevance > b.relevance;});

    // for(Xapian::MSetIterator m = mset.begin(); m != mset.end(); ++m)
    const int limit_show = 5;
    int count = 0;    
    for(auto it= search_results.begin(); it!=search_results.end(); ++it)
    {
        std::string title = it->it.get_document().get_value(SLOT_TITLE);
        std::string url = it->it.get_document().get_value(SLOT_URL);

        std::cout << title << '\n';
        std::cout << "relevance:" << it->relevance << "\n";
        std::cout << url << '\n';

        const std::string &data = it->it.get_document().get_data();
        std::string snippet =
            Trokam::PlainTextProcessor::snippet(data, querystring, 250);
        boost::replace_all(snippet, "   ", " ");
        boost::replace_all(snippet, "  ", " ");

        std::cout << snippet << "\n\n";
        std::cout << '\n';

        count++;
        if(count > limit_show)
        {
            break;
        }
    }

    std::cout << '\n';
}

void Trokam::Grasp::clean()
{
    Trokam::FileOps::rmDir(db_path);
}
