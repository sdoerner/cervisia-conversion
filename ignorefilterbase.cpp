/* 
 *  Copyright (C) 2004-2005 Christian Loose <christian.loose@kdemail.net>
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

#include "ignorefilterbase.h"
using namespace Cervisia;

#include <qfile.h>
#include <qstringlist.h>
#include <qtextstream.h>


IgnoreFilterBase::IgnoreFilterBase(IgnoreFilterBase* nextFilter)
    : m_nextFilter(nextFilter)
{
}


IgnoreFilterBase::~IgnoreFilterBase()
{
    delete m_nextFilter;
    m_nextFilter = 0;
}


bool IgnoreFilterBase::matches(const QString& fileName) const
{
    bool result = false;

    if( m_nextFilter )
        result = m_nextFilter->matches(fileName);

    return doMatches(fileName) || result;
}


void IgnoreFilterBase::addEntriesFromString(const QString& str)
{
    QStringList entries = QStringList::split(' ', str);
    for( QStringList::iterator it = entries.begin(); it != entries.end(); ++it )
    {
        addEntry(*it);
    }
}


void IgnoreFilterBase::addEntriesFromFile(const QString& name)
{
    QFile file(name);

    if( file.open(IO_ReadOnly) )
    {
        QTextStream stream(&file);
        while( !stream.eof() )
        {
            addEntriesFromString(stream.readLine());
        }
    }
}