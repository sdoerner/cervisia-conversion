/*
 * Copyright (c) 2005,2006 André Wöbbeking <Woebbeking@web.de>
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


#include "svn_annotate_parser.h"


namespace Cervisia
{


SvnAnnotateParser::SvnAnnotateParser(QObject* parent)
    : AnnotateParser(parent, "SvnAnnotateParser")
{
}


void SvnAnnotateParser::parseLine(const QString& line)
{
    AnnotateInfo info;
    info.m_revision = line.left(8).stripWhiteSpace();
    info.m_line     = line.mid(18);

    push_back(info);
}


} // namespace Cervisia
