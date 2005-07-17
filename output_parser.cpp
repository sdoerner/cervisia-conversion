/*
 * Copyright (c) 2005 André Wöbbeking <Woebbeking@web.de>
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
 * Foundation, Inc., 51 Franklin Steet, Fifth Floor, Cambridge, MA 02110-1301, USA.
 */


#include "output_parser.h"


namespace Cervisia
{


void OutputParser::parseOutput(const QString& output)
{
    const QString text(restOutput.isEmpty() ? output : restOutput + output);

    int lastPos(0);
    int pos;
    while ((pos = text.find('\n', lastPos)) >= 0)
    {
        const QString line(text.mid(lastPos, pos - lastPos));

        parseLine(line);

        lastPos = pos + 1;
    }

    restOutput = text.mid(lastPos);
}


} // namespace Cervisia


#include "output_parser.moc"
