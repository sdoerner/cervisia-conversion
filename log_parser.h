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


#ifndef CERVISIA_LOG_PARSER_H
#define CERVISIA_LOG_PARSER_H


#include "output_parser.h"

#include "loginfo.h"

#include <qvaluelist.h>


namespace Cervisia
{



/**
 * Base class to parse output of a log command.
 */
class LogParser : public OutputParser
{
public:

    typedef QValueList<LogInfo> TLogInfoSeq;



    /**
     * @return The list of parsed LogInfos.
     */
    const TLogInfoSeq& logInfos() const;

protected:

    /**
     * Add one LogInfo to the list of LogInfos.
     *
     * @param logInfo The info to add.
     */
    void push_back(const LogInfo& logInfo);

private:

    TLogInfoSeq m_logInfos;
};


} // namespace Cervisia


#endif // CERVISIA_LOG_PARSER_H
