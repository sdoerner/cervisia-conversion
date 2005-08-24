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


#ifndef CERVISIA_DIFF_INFO_H
#define CERVISIA_DIFF_INFO_H


#include <qstring.h>
#include <qvaluelist.h>


namespace Cervisia
{


/**
 * Dumb data struct to store the results of the diff command. An instance
 * of this struct represents a line of the diff output.
 */
struct DiffInfo
{
    enum Type
    {
        /**
         * The first lines until a line beginning with "+++".
         **/
        Header,

        /**
         * A line like "^@@ -([0-9]+),([0-9]+) \\+([0-9]+),([0-9]+) @@.*$".
         **/
        Region,

        /**
         * A unchanged line (beginning with ' ').
         **/
        Unchanged,

        /**
         * A removed line (beginning with '-').
         **/
        Removed,

        /**
         * An added line (beginning with '+').
         **/
        Added
    };

    /**
     * The content of the line. For lines of type Unchanged, Removed and Added
     * the first character is removed.
     */
    QString m_line;

    /**
     * The type of the line.
     */
    Type m_type;
};


typedef QValueList<DiffInfo> DiffInfoList;


} // namespace Cervisia


#endif // CERVISIA_DIFF_INFO_H
