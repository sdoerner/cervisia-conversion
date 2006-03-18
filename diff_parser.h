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


#ifndef CERVISIA_DIFF_PARSER_H
#define CERVISIA_DIFF_PARSER_H


#include "output_parser.h"

#include "diff_info.h"


namespace Cervisia
{


/**
 * Base class to parse output of a diff command.
 */
class DiffParser : public OutputParser
{
public:

    explicit DiffParser(QObject* parent = 0, const char* name = 0);

    /**
     * @return The list of parsed DiffInfos.
     */
    const DiffInfoList& diffInfos() const;

private:

    virtual void parseLine(const QString& line);

    enum EState
    {
        Header,
        Lines
    };

    DiffInfoList m_diffInfos;

    EState m_state;
};


} // namespace Cervisia


#endif // CERVISIA_DIFF_PARSER_H
