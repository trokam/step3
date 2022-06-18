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

#ifndef TROKAM_COMMON_H
#define TROKAM_COMMON_H

/**
 * Configuration files.
 **/

const std::string CONFIG_FILE= "/usr/local/etc/trokam.cfg";
const std::string CLUSTER_FILE= "/usr/local/etc/cluster.cfg";

/**
 * Processing parameters.
 **/

const int SEQUENCE_SIZE =             5;
const int WORDS_LIMIT =            5000;
const int TEXT_LIMIT =    4*WORDS_LIMIT;
const int SEQUENCE_LIMIT =           12;
const int RELEVANCE_THRESHOLD=      400;

const int DOMAIN_URL_LIMIT =        100;
const int PAGE_URL_LIMIT =          100;

const int DOMAIN_INDEX =              1;
const int DOMAIN_REJECT =             2;

const int PAGE_INDEX =                3;
const int PAGE_REJECT =               4;

const int DOMAIN_INDEXED =            3;
const int INDEX_HTTP =                1;
const int INDEX_HTTPS =               2;

/**
 * Object reference.
 **/

const int GENERAL_INFO =    0;
const int SMALL_LOGO =      1;
const int BIG_LOGO =        2;
const int BRIEF_INTRO =     4;
const int SEARCH_STATE =    5;

/**
 * Database IDs.
 **/

const int DB_TEXT_SEARCH =            7;
const int DB_CONTROL =               11;
const int DB_DEPOT =                 13;

/**
 * String literals.
 **/

const std::string HTTP =             "http://";
const std::string HTTPS =            "https://";
const std::string SLASH =            "/";
const std::string EMPTY =            "";

const std::string HTML =             "text/htm";
const std::string XML =              "text/xml";

const std::string PAGE_UNIQUE =      "page_unique";
const std::string SEQ =              "seq:";
const std::string TEXT_DB =          "textDB";

const std::string VOID =             "(void)";

const std::string PAGE_CRUNCHER =    "page_cruncher";
const std::string DOMAIN_CRUNCHER =  "domain_cruncher";

/**
 * Warnings
 **/

const std::string WARNING_APPROOT_EMPTY =         "warning: the approot is empty, it looks suspect.";

/**
 * Errors.
 **/

const std::string ERROR_NO_PAGES_TO_PROCESS =            "error: there are no pages to process.";
const std::string ERROR_DOWNLOAD_FAIL =                  "error: download fail.";
const std::string ERROR_TYPE_NOT_SUPPORTED =             "error: file type not supported.";
const std::string ERROR_EXTRACTING_CONTENT_FAIL =        "error: extracting content fail.";
const std::string ERROR_EXTRACTING_LINKS_FAIL =          "error: extracting links fail.";
const std::string ERROR_COULD_NOT_CONNECT_TO_DATABASE =  "error: could not connect to db server.";

/**
 * State of page processing.
 **/

const int UNKNOWN =                     -1;
const int SUCCESS =                      0;
const int NO_PAGES_TO_PROCESS =          1;
const int DOWNLOAD_FAIL =                2;
const int TYPE_NOT_SUPPORTED =           3;
const int EXTRACTING_CONTENT_FAIL =      4;
const int EXTRACTING_LINKS_FAIL =        5;
const int COULD_NOT_CONNECT_TO_DATABASE= 6;

#endif /// TROKAM_COMMON_H
