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
#include <vector>
#include <utility>

// Trokam
#include "readable_content_db.h"
#include "file_ops.h"
#include "plain_text_processor.h"

void Trokam::ReadableContentDB::open(const std::string &path)
{
    db.reset(new Xapian::Database(path));
}

void Trokam::ReadableContentDB::add(const std::string &path)
{
    Xapian::Database database(path);
    db->add_database(database);
}

std::vector<Trokam::Finding>
    Trokam::ReadableContentDB::search(
        const std::string &querystring,
        const std::vector<std::string> &languages,
        Xapian::doccount results_requested)
{
    const Xapian::doccount max_requested = 50;
    const Xapian::doccount pre_scan_size = 250;

    if(results_requested > max_requested)
    {
        results_requested = max_requested;
    }

    // Set up a QueryParser.
    Xapian::QueryParser queryparser;
    queryparser.set_stemming_strategy(queryparser.STEM_SOME);

    // And parse the query.
    Xapian::Query query = queryparser.parse_query(querystring);

    // Build a query for languages.
    std::vector<Xapian::Query> lang_queries;
    for(const auto &element: languages)
    {
        std::string lang_term = "L";
        lang_term += element;
        lang_queries.push_back(Xapian::Query(lang_term));
    }

    // Combine these queries with an 'or' operator.
    Xapian::Query language_query(
        Xapian::Query::OP_OR,
        lang_queries.begin(),
        lang_queries.end());

    // Use the language query to filter the main query.
    query = Xapian::Query(
        Xapian::Query::OP_FILTER,
        query,
        language_query);

    // Use an Enquire object on the database to run the query.
    Xapian::Enquire enquire(*db);
    enquire.set_query(query);

    std::vector<Trokam::Finding> preliminar;

    const Xapian::doccount offset = 1;
    Xapian::MSet mset = enquire.get_mset(offset, pre_scan_size);
    for(Xapian::MSetIterator m = mset.begin(); m != mset.end(); ++m)
    {
        Trokam::Finding finding;

        const std::string &title = m.get_document().get_value(SLOT_TITLE);
        const std::string &url   = m.get_document().get_value(SLOT_URL);
        const std::string &data = m.get_document().get_data();

        finding.title           = title;
        finding.url             = url;
        finding.relevance_body  = m.get_weight();
        finding.relevance_url   = Trokam::PlainTextProcessor::howMuchOf(url, querystring);
        finding.relevance_title = Trokam::PlainTextProcessor::howMuchOf(title, querystring);
        finding.relevance_total =
            finding.relevance_body +
            finding.relevance_url * 24.0 +
            finding.relevance_title * 3.5;
        finding.snippet =
            mset.snippet(
                data,
                300,
                Xapian::Stem(),
                Xapian::MSet::SNIPPET_BACKGROUND_MODEL | Xapian::MSet::SNIPPET_EXHAUSTIVE,
                std::string("<strong>"),
                std::string("</strong>"),
                std::string("..."));

        preliminar.push_back(finding);
    }

    std::sort(
        preliminar.begin(),
        preliminar.end(),
        [](auto a, auto b) { return a.url < b.url; });

    std::vector<Trokam::Finding> result;

    std::string previous_url;
    for(auto it=preliminar.begin(); it!=preliminar.end(); it++)
    {
        std::string &current_url = it->url;
        if(current_url != previous_url)
        {
           result.push_back(*it);
        }
        previous_url = current_url;
    }

    std::sort(
        result.begin(),
        result.end(),
        [](auto a, auto b) { return a.relevance_total > b.relevance_total; });

    if(result.size() > results_requested)
    {
        result.resize(results_requested);
    }

    return result;
}

std::vector<std::pair<std::string, Xapian::doccount>>
    Trokam::ReadableContentDB::lookUp(
        const std::string &partial_term)
{
    std::vector<std::pair<std::string, Xapian::doccount>> result;
    Xapian::TermIterator it = db->allterms_begin(partial_term);
    Xapian::TermIterator end = db->allterms_end(partial_term);

    while(it != end)
    {
        auto strike = std::make_pair(*it, it.get_termfreq());
        result.push_back(strike);
        it++;
    }

    std::sort(
        result.begin(),
        result.end(),
        [](auto a, auto b)
            {
                return std::get<Xapian::doccount>(a) > std::get<Xapian::doccount>(b);
            }
        );

    return result;
}

void Trokam::ReadableContentDB::close()
{
    if(db.get()!= nullptr)
    {
        db->close();
    }
}
