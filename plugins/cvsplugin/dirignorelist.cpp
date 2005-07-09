/* 
 *  Copyright (C) 2004 Christian Loose <christian.loose@kdemail.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "dirignorelist.h"
using namespace Cervisia;


DirIgnoreList::DirIgnoreList(const QString& path, IgnoreFilterBase* nextFilter)
    : IgnoreFilterBase(nextFilter)
{
    addEntriesFromFile(path + "/.cvsignore");
}


bool DirIgnoreList::doMatches(const QString& fileName) const
{
    return m_stringMatcher.match(fileName);
}


void DirIgnoreList::addEntry(const QString& entry)
{
    if (entry != QChar('!'))
    {
        m_stringMatcher.add(entry);
    }
    else
    {
        m_stringMatcher.clear();
    }
}
