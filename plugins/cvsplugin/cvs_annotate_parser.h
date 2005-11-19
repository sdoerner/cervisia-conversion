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


#ifndef CERVISIA_CVS_ANNOTATE_PARSER_H
#define CERVISIA_CVS_ANNOTATE_PARSER_H


#include "annotate_parser.h"


namespace Cervisia
{


/**
 * Class to parse output of the CVS annotate command.
 */
class CvsAnnotateParser : public AnnotateParser
{
public:

    explicit CvsAnnotateParser(QObject* parent = 0);

private:

    virtual void parseLine(const QString& line);

    void parseInfos(const QString& line);

    enum EState
    {
        Begin,
        Infos
    };

    EState m_state;
};


} // namespace Cervisia


#endif // CERVISIA_CVS_ANNOTATE_PARSER_H
