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


#include "svn_log_parser.h"

#include <krfcdate.h>

#include <qstringlist.h>


namespace Cervisia
{


SvnLogParser::SvnLogParser()
    : m_state(Begin)
{
}


void SvnLogParser::parseLine(const QString& line)
{
    switch (m_state)
    {
    case Begin:
        if (line == "------------------------------------------------------------------------")
            m_state = Infos;
        break;

    case Infos:
        parseInfos(line);
        break;

    case Comment:
        parseComment(line);
        break;
    }
}


void SvnLogParser::parseComment(const QString& line)
{
    if (line == "------------------------------------------------------------------------")
        m_state = Infos;

    if (m_state == Comment) // still in message
    {
        m_logInfo.m_comment += '\n' + line;
    }
    else
    {
        if (m_logInfo.m_comment.startsWith("\n\n"))
            m_logInfo.m_comment.remove(0, 2);
        else if (m_logInfo.m_comment.startsWith("\n"))
            m_logInfo.m_comment.remove(0, 1);
        if (m_logInfo.m_comment.endsWith("\n"))
            m_logInfo.m_comment.truncate(m_logInfo.m_comment.length() - 1);

        push_back(m_logInfo);

        // reset for next entry
        m_logInfo = LogInfo();
    }
}


void SvnLogParser::parseInfos(const QString& line)
{
    // allow empty entries as the author column can be empty
    const QStringList strlist(QStringList::split(" | ", line, true));

    m_logInfo.m_revision = strlist[0].mid(1);

    m_logInfo.m_author = strlist[1];

    // convert date into ISO format (YYYY-MM-DDTHH:MM:SS)
    QString dateTimeStr(strlist[2]);
    // remove textual date
    const int posTextDate(dateTimeStr.find(" ("));
    if (posTextDate >= 0)
        dateTimeStr = dateTimeStr.left(posTextDate);
    // use ISO separator
    const int posSep(dateTimeStr.find(' '));
    if (posSep >= 0)
        dateTimeStr.replace(posSep, 1, 'T');

    m_logInfo.m_dateTime.setTime_t(KRFCDate::parseDateISO8601(dateTimeStr));

    m_state = Comment;
}


} // namespace Cervisia
