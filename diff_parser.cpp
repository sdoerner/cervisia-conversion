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


#include "diff_parser.h"


namespace Cervisia
{


DiffParser::DiffParser(QObject* parent, const char* name)
    : OutputParser(parent, name),
      m_state(Header)
{
}


const DiffInfoList& DiffParser::diffInfos() const
{
    return m_diffInfos;
}


void DiffParser::parseLine(const QString& line)
{
    DiffInfo diffInfo;
    switch (m_state)
    {
    case Header:
        diffInfo.m_line = line;
        diffInfo.m_type = DiffInfo::Header;

        if (line.startsWith("+++"))
            m_state = Lines;
        break;

    case Lines:
        if (line.startsWith("@@"))
        {
            diffInfo.m_line = line;
            diffInfo.m_type = DiffInfo::Region;
        }
        else
        {
            // can this happen?
            if (line.isEmpty())
                return;

            diffInfo.m_line = line.mid(1);
            switch (line[0].latin1())
            {
            case '-':
                diffInfo.m_type = DiffInfo::Removed;
                break;

            case '+':
                diffInfo.m_type = DiffInfo::Added;
                break;

            default:
                diffInfo.m_type = DiffInfo::Unchanged;
                break;
            }
        }
        break;
    };

    m_diffInfos.push_back(diffInfo);
}


} // namespace Cervisia
