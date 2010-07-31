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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "ignorelistbase.h"
using namespace Cervisia;

#include <tqfile.h>
#include <tqstringlist.h>
#include <tqtextstream.h>


void IgnoreListBase::addEntriesFromString(const TQString& str)
{
    TQStringList entries = TQStringList::split(' ', str);
    for( TQStringList::iterator it = entries.begin(); it != entries.end(); ++it )
    {
        addEntry(*it);
    }
}


void IgnoreListBase::addEntriesFromFile(const TQString& name)
{
    TQFile file(name);

    if( file.open(IO_ReadOnly) )
    {
        TQTextStream stream(&file);
        while( !stream.eof() )
        {
            addEntriesFromString(stream.readLine());
        }
    }
}
