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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */


#ifndef CERVISIA_CVS_LOG_PARSER_H
#define CERVISIA_CVS_LOG_PARSER_H


#include "log_parser.h"


namespace Cervisia
{


/**
 * Class to parse output of the CVS log command.
 */
class CvsLogParser : public LogParser
{
public:

    explicit CvsLogParser(QObject* parent = 0);

private:

    virtual void parseLine(const QString& line);

    void parseComment(const QString& line);

    void parseInfos(const QString& line);

    void parseTag(const QString& line);

    enum EState
    {
        Begin,
        Tags,
        Admin,
        Revision,
        Infos,
        Branches,
        Comment,
        Finished
    };

    EState m_state;

    LogInfo m_logInfo;

    QString m_rev;

    struct Tag
    {
        QString rev;
        QString name;
        QString branchpoint;
    };

    typedef QValueList<Tag> TTagSeq;
    TTagSeq m_tags;
};


} // namespace Cervisia


#endif // CERVISIA_CVS_LOG_PARSER_H
