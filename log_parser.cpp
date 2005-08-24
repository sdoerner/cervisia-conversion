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


#include "log_parser.h"


namespace Cervisia
{


LogParser::LogParser(QObject* parent, const char* name)
    : OutputParser(parent, name)
{
}


const LogInfoList& LogParser::logInfos() const
{
    return m_logInfos;
}


void LogParser::push_back(const LogInfo& logInfo)
{
    m_logInfos.push_back(logInfo);
}


} // namespace Cervisia
