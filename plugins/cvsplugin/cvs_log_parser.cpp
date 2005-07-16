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


#include "cvs_log_parser.h"

#include <krfcdate.h>

#include <qstringlist.h>


namespace Cervisia
{


CvsLogParser::CvsLogParser()
    : m_state(Begin)
{
}


void CvsLogParser::parseLine(const QString& line)
{
    switch (m_state)
    {
    case Begin:
        if (line == "symbolic names:")
            m_state = Tags;
        break;

    case Tags:
        parseTag(line);
        break;

    case Admin:
        if (line == "----------------------------")
            m_state = Revision;
        break;

    case Revision:
        m_logInfo.m_revision = m_rev = line.section(' ', 1, 1);
        m_state = Infos;
        break;

    case Infos:
        parseInfos(line);
        break;

    case Branches:
        if (!line.startsWith("branches:"))
        {
            m_logInfo.m_comment = line;
            m_state = Comment;
        }
        break;

    case Comment:
        parseComment(line);
        break;

    case Finished:
        ;
    }
}


void CvsLogParser::parseComment(const QString& line)
{
    if (line == "----------------------------")
        m_state = Revision;
    else if (line == "=============================================================================")
        m_state = Finished;

    if (m_state == Comment) // still in message
    {
        m_logInfo.m_comment += '\n' + line;
    }
    else
    {
        // Create tagcomment
        QString branchrev;
        int pos1, pos2;
        // 1.60.x.y => revision belongs to branch 1.60.0.x
        if ((pos2 = m_rev.findRev('.')) > 0
            && (pos1 = m_rev.findRev('.', pos2 - 1)) > 0)
            branchrev = m_rev.left(pos2);

        // Build TagInfo for LogInfo
        for (TTagSeq::const_iterator itTag(m_tags.begin()),
                                     itTagEnd(m_tags.end());
             itTag != itTagEnd; ++itTag)
        {
            const Tag& tag(*itTag);

            if (m_rev == tag.rev)
                // This never matches branch tags...
                m_logInfo.m_tags.push_back(TagInfo(tag.name, TagInfo::Tag));

            if (m_rev == tag.branchpoint)
                m_logInfo.m_tags.push_back(TagInfo(tag.name, TagInfo::Branch));

            if (branchrev == tag.rev)
                // ... and this never matches ordinary tags :-)
                m_logInfo.m_tags.push_back(TagInfo(tag.name, TagInfo::OnBranch));
        }

        push_back(m_logInfo);

        // reset for next entry
        m_logInfo = LogInfo();
    }
}


void CvsLogParser::parseInfos(const QString& line)
{
    const QStringList strlist(QStringList::split(';', line));

    // convert date into ISO format (YYYY-MM-DDTHH:MM:SS)
    const int len(strlist[0].length());
    QString dateTimeStr(strlist[0].right(len - 6)); // remove 'date: '
    dateTimeStr.replace('/', '-');

    const QString date(dateTimeStr.section(' ', 0, 0));
    const QString time(dateTimeStr.section(' ', 1, 1));
    m_logInfo.m_dateTime.setTime_t(KRFCDate::parseDateISO8601(date + 'T' + time));

    m_logInfo.m_author = strlist[1].section(':', 1, 1).stripWhiteSpace();

    m_state = Branches;
}


void CvsLogParser::parseTag(const QString& line)
{
    if (!line.isEmpty() && line[0] == '\t')
    {
        const QStringList strlist(QStringList::split(':', line));
        QString rev(strlist[1].simplifyWhiteSpace());
        const QString tagName(strlist[0].simplifyWhiteSpace());

        QString branchpoint;
        int pos1, pos2;
        if ((pos2 = rev.findRev('.')) > 0
            && (pos1 = rev.findRev('.', pos2 - 1)) > 0
            && rev.mid(pos1 + 1, pos2 - pos1 - 1) == "0")
        {
            // For a branch tag 2.10.0.6, we want:
            // branchpoint = "2.10"
            // rev = "2.10.6"
            branchpoint = rev.left(pos1);
            rev.remove(pos1 + 1, pos2 - pos1);
        }

        if (rev != "1.1.1")
        {
            Tag tag;
            tag.rev = rev;
            tag.name = tagName;
            tag.branchpoint = branchpoint;
            m_tags.push_back(tag);
        }
    }
    else
    {
        m_state = Admin;
    }
}


} // namespace Cervisia
