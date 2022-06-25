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
#include "readable_content_db.h"
#include "file_ops.h"
#include "plain_text_processor.h"

Trokam::ReadableContentDB::ReadableContentDB()
{
    // std::string dbpath;

    /**
    if(const char* env_home = std::getenv("HOME"))
    {
        // std::cout << "Your PATH is: " << env_p << '\n';
        db_path  = env_home;
        db_path += "/ReadableContentDB";
    }
    else
    {
        std::cerr << "fail: could not create ReadableContentDB database.";
        exit(1);
    }
    **/

    db_path = "/usr/local/data/trokam/grasp";

    db.reset(
        new Xapian::Database(db_path));
}

std::vector<Trokam::Finding>
    Trokam::ReadableContentDB::search(
        const std::string &querystring,
        const std::string &languages,
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

        // For debugging
        std::cout << "languages:'" <<languages << "'\n";

        std::vector<std::string> lang_enquiry;
        if(languages.empty())
        {
            lang_enquiry.push_back("english");
        }
        else
        {
            lang_enquiry =
                Trokam::PlainTextProcessor::tokenize(languages, ',');
        }

        // For debugging
        for(const std::string &e: lang_enquiry)
        {
            std::cout << "lang:'" << e << "'\n";
        }
        std::cout << '\n';

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

        float relevance =
            m.get_weight() * title_weight * title_weight * url_weight * url_weight;
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

    std::vector<Trokam::Finding> result;
    for(auto it= search_results.begin(); it!=search_results.end(); ++it)
    {
        Trokam::Finding finding;

        finding.title     = it->it.get_document().get_value(SLOT_TITLE);
        finding.url       = it->it.get_document().get_value(SLOT_URL);
        finding.relevance = it->relevance;

        const std::string &data = it->it.get_document().get_data();
        finding.snippet =
            Trokam::PlainTextProcessor::snippet(data, querystring, 250);

        result.push_back(finding);
    }

    return result;
}
