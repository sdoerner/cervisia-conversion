/* 
 *  Copyright (C) 2005 Christian Loose <christian.loose@kdemail.net>
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

#include "globalignorelist.h"
using namespace Cervisia;

#include <qdir.h>
#include <kdebug.h>

#include "stringmatcher.h"


struct Data
{
    static StringMatcher m_stringMatcher;
    static bool          m_isInitialized;
};

StringMatcher Data::m_stringMatcher;
bool          Data::m_isInitialized = false;


GlobalIgnoreList::GlobalIgnoreList(IgnoreFilterBase* nextFilter)
    : IgnoreFilterBase(nextFilter)
{
    if( !Data().m_isInitialized )
        setup();
}

bool GlobalIgnoreList::doMatches(const QString& fileName) const
{
    return Data().m_stringMatcher.match(fileName);
}


void GlobalIgnoreList::addEntry(const QString& entry)
{
    Data().m_stringMatcher.add(entry);
}


void GlobalIgnoreList::setup()
{
    static const char ignorestr[] = ". .. .svn *.o *.lo *.la #*# .*.rej *.rej .*~ *~ .#* .DS_Store";

    addEntriesFromString(QString::fromLatin1(ignorestr));
//     addEntriesFromFile(QDir::homeDirPath() + "/.cvsignore");

    Data().m_isInitialized = true;
}
