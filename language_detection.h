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

namespace Trokam
{
    class LanguageDetection
    {
        public:

            LanguageDetection()
            {
                lang_occ.resize(12);
                lang_occ[unknown] = 0;
                lang_occ[english] = 0;
                lang_occ[japanese] = 0;
                lang_occ[castillian] = 0;
                lang_occ[german] = 0;
                lang_occ[russian] = 0;
                lang_occ[french] = 0;
                lang_occ[mandarin] = 0;
                lang_occ[italian] = 0;
                lang_occ[portuguese] = 0;
                lang_occ[polish] = 0;
                lang_occ[dutch] = 0;
            }

            std::string detectLanguage(
                const std::string &content)
            {
                // log(debug) << "detect_lang -- begin\n";
                // std::cout.flush();

                std::map<std::string, int> fragment_stat;
                int count = 0;
                std::string::size_type ini= 0;
                std::string::size_type end= 0;

                while((ini < content.size()) && (end != std::string::npos) && (count < 100))
                {
                    end= content.find(' ', ini);

                    // std::cout << "ini=" << ini << "\n";
                    // std::cout << "end=" << end << "\n";
                    // std::cout << "count=" << count << "\n";

                    if((ini != std::string::npos) && (end != std::string::npos))
                    {
                        const std::string::size_type len = end - ini;
                        if((len > 0) && (len < 4))
                        {
                            std::string fragment = content.substr(ini, len);
                            boost::algorithm::trim_if(fragment, boost::algorithm::is_any_of("\n\r\t/1234567890$¿?¡!{}[]().,;:|-='\\\""));
                            if(!fragment.empty())
                            {
                                boost::algorithm::to_lower(fragment);
                                // std::cout << "fragment:'" << fragment << "'\n";
                                fragment_stat[fragment] += 1;
                                count++;
                            }
                        }
                        ini = end + 1;
                    }
                }

                /*
                for(auto it = fragment_stat.begin(); it != fragment_stat.end(); ++it)
                {
                    std::cout << "no sorted:" <<  it->first << '\t' << it->second << '\n';
                }
                */

                // LanguageDetection lang_det;
                // std::cout << "language:" << lang_det.lang_matching(fragment_stat) << "\n";

                // log(debug) << "detect_lang -- end\n";
                // std::cout.flush();

                return lang_matching(fragment_stat);
            }

            std::string lang_matching(const std::map<std::string, int> &fragment_stat)
            {
                std::vector<std::pair<std::string, int>> word_occ(fragment_stat.begin(), fragment_stat.end());
                std::sort(word_occ.begin(), word_occ.end(),
                [](std::pair<std::string, int> a, std::pair<std::string, int> b)
                                                        {
                                                            return a.second > b.second;
                                                        });

                int count = 0;
                for (std::size_t i=0; i<word_occ.size(); i++)
                {
                    // std::cout << "sorted:" << word_occ[i].first << "\t" << word_occ[i].second << "\n";

                    std::string &word= word_occ[i].first;

                    if ((word == "the") || (word == "of") || (word == "and") || (word == "to") || (word == "a"))
                    {
                        lang_occ[english] += word_occ[i].second;
                    }
                    else if ((word == "der") || (word == "und") || (word == "die") || (word == "von") || (word == "zu"))
                    {
                        lang_occ[german] += word_occ[i].second;
                    }
                    else if (word == "in")
                    {
                        lang_occ[german] += word_occ[i].second;
                        lang_occ[english] += word_occ[i].second;
                        lang_occ[dutch] += word_occ[i].second;
                    }
                    else if ((word == "de") || (word == "la") || (word == "en"))
                    {
                        lang_occ[castillian] += word_occ[i].second;
                        lang_occ[french] += word_occ[i].second;
                        lang_occ[dutch] += word_occ[i].second;
                    }
                    else if ((word == "el") || (word == "y") || (word == "que") || (word == "los") || (word == "con"))
                    {
                        lang_occ[castillian] += word_occ[i].second;
                    }
                    else if ((word == "des") || (word == "et") || (word == "les") || (word == "le") || (word == "il"))
                    {
                        lang_occ[french] += word_occ[i].second;
                    }
                    else if ((word == "van") || (word == "zijn") || (word == "hij") || (word == "een") || (word == "het"))
                    {
                        lang_occ[dutch] += word_occ[i].second;
                    }
                    else
                    {
                        lang_occ[unknown] += word_occ[i].second;
                    }

                    count++;
                    if (count > 6) break;
                }

                int max= 0;
                int lang= 0;
                for(int i=1; i<12; i++)
                {
                    if(lang_occ[i] > max)
                    {
                        max = lang_occ[i];
                        lang = i;
                    }
                }

                switch (lang)
                {
                    case 1:
                        return "english";
                        break;

                    case 3:
                        return "castillian";
                        break;

                    case 4:
                        return "german";
                        break;

                    case 6:
                        return "french";
                        break;

                    case 11:
                        return "dutch";
                        break;

                    default:
                        return "unknown";
                        break;
                }

                return "unknown";
            }

        private:

            const int unknown = 0;
            const int english = 1;
            const int japanese = 2;
            const int castillian = 3;
            const int german = 4;
            const int russian = 5;
            const int french = 6;
            const int mandarin = 7;
            const int italian  = 8;
            const int portuguese = 9;
            const int polish = 10;
            const int dutch = 11;

            std::vector<int> lang_occ;
    };
}
