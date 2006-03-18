/*
 * Copyright (c) 2005 Christian Loose <christian.loose@kdemail.net>
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

#include "cvs_update_parser.h"
using Cervisia::CvsUpdateParser;

#include <kdebug.h>


CvsUpdateParser::CvsUpdateParser()
    : UpdateParser()
{
}


CvsUpdateParser::~CvsUpdateParser()
{
}


/**
 * Process one line from the output of 'cvs update'. If isSimulation()
 * is true, it is assumed that the output is from a command
 * 'cvs update -n', i.e. cvs actually changes no files.
 */
void CvsUpdateParser::parseLine(const QString& line)
{
    kdDebug(8050) << k_funcinfo << "line = " << line << endl;

    if( line.length() > 2 && line[1] == ' ' )
    {
        EntryStatus status = Cervisia::Unknown;
        switch (line[0].latin1())
        {
        case 'C':
            status = Cervisia::Conflict;
            break;
        case 'A':
            status = Cervisia::LocallyAdded;
            break;
        case 'R':
            status = Cervisia::LocallyRemoved;
            break;
        case 'M':
            status = Cervisia::LocallyModified;
            break;
        case 'U':
            status = isSimulation() ? Cervisia::NeedsUpdate : Cervisia::Updated;
            break;
        case 'P':
            status = isSimulation() ? Cervisia::NeedsPatch : Cervisia::Patched;
        case '?':
            status = Cervisia::NotInCVS;
            break;
        default:
            return;
        }

        emit updateItemStatus(line.mid(2).stripWhiteSpace(), status, false);
    }

    const QString removedFileStart(QString::fromLatin1("cvs server: "));
    const QString removedFileEnd(QString::fromLatin1(" is no longer in the repository"));
    if (line.startsWith(removedFileStart) && line.endsWith(removedFileEnd))
    {
    }

#if 0
    else if (str.left(21) == "cvs server: Updating " ||
             str.left(21) == "cvs update: Updating ")
        updateItem(str.right(str.length()-21), Unknown, true);
#endif
}
