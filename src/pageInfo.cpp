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

/// Trokam
#include "pageInfo.h"
#include "common.h"

Trokam::PageInfo::PageInfo()
{
    index= -1;
    domainIndex= -1;
    domainType= DOMAIN_REJECT;
    pageType= PAGE_REJECT;
    size= 0;
    complexity= 0.0;
}

Trokam::PageInfo::PageInfo(const Trokam::PageInfo &value)
{
    index= value.index;
    urlOrigin= value.urlOrigin;
    urlForwarded= value.urlForwarded;
    domainIndex= value.domainIndex;
    domainType= value.domainType;
    pageType= value.pageType;
    raw= value.raw;
    content= value.content;
    links= value.links;
    title= value.title;
    type= value.type;
    size= value.size;
    complexity= value.complexity;
    sequences= value.sequences;
    // urlBag= value.urlBag;
}

/**
 * Copy constructor.
 **/
Trokam::PageInfo& Trokam::PageInfo::operator= (const Trokam::PageInfo &value)
{
    /**
     * Avoid self-assignment.
     */
    if(this == &value)
    {
        return *this;
    }

    /**
     * Copy data.
     */
    index= value.index;
    urlOrigin= value.urlOrigin;
    urlForwarded= value.urlForwarded;
    domainIndex= value.domainIndex;
    domainType= value.domainType;
    pageType= value.pageType;
    raw= value.raw;
    content= value.content;
    links= value.links;
    title= value.title;
    type= value.type;
    size= value.size;
    complexity= value.complexity;
    sequences= value.sequences;
    // urlBag= value.urlBag;

    /**
     * Return 'this' object.
     */
    return *this;
}
