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
 * Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "svn_update_parser.h"
using Cervisia::SvnUpdateParser;

#include <qregexp.h>
#include <kdebug.h>


SvnUpdateParser::SvnUpdateParser()
    : UpdateParser()
{
}


SvnUpdateParser::~SvnUpdateParser()
{
}


void SvnUpdateParser::parseLine(const QString& line)
{
    if( isSimulation() )
        parseStatusLine(line);
    else
        parseUpdateLine(line);
}


void SvnUpdateParser::parseStatusLine(const QString& line)
{
    kdDebug(8050) << k_funcinfo << "line = " << line << endl;

    QRegExp rx(".*Revision.*\\d+");
    rx.setCaseSensitive(false);
    if( line.length() > 2 && rx.search(line) < 0 )
    {
        QString fileName = line.right(line.length() - 20);

        kdDebug(8050) << k_funcinfo << "fileName = " << fileName
                      << ", length = " << line.length() << endl;

        EntryStatus status = Cervisia::Unknown;
        switch( line[0].latin1() )
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
            case '?':
                status = Cervisia::NotInCVS;
                break;
            default:
                if( line[7].latin1() == '*' )
                    status = Cervisia::NeedsUpdate;
                else
                    return;
        }

        emit updateItemStatus(fileName, status, false);
    }
}


void SvnUpdateParser::parseUpdateLine(const QString& line)
{
    kdDebug(8050) << k_funcinfo << "line = " << line << endl;

    QRegExp rx(".*Revision.*\\d+");
    rx.setCaseSensitive(false);
    if( line.length() > 2 && rx.search(line) < 0 )
    {
        QString fileName = line.right(line.length() - 5);

        kdDebug(8050) << k_funcinfo << "fileName = " << fileName
                      << ", length = " << line.length() << endl;

        EntryStatus status = Cervisia::Unknown;
        switch( line[0].latin1() )
        {
            case 'C':
                status = Cervisia::Conflict;
                break;
            case 'A':
                status = Cervisia::LocallyAdded;
                break;
            case 'D':
                status = Cervisia::LocallyRemoved;
                break;
            case 'G':
                status = Cervisia::Merged;
                break;
            case 'U':
                status = Cervisia::Updated;
                break;
            case '?':
                status = Cervisia::NotInCVS;
                break;
            default:
                return;
        }

        emit updateItemStatus(fileName, status, false);
    }
}
