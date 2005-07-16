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


#ifndef CERVISIA_OUTPUT_PARSER_H
#define CERVISIA_OUTPUT_PARSER_H


#include <qobject.h>


namespace Cervisia
{


/**
 * Base class to parse any kind of output line by line.
 */
class OutputParser : public QObject
{
    Q_OBJECT

public slots:

    /**
     * Call this slot with any output. The output will be splitted into single
     * lines and for each line parseLine() is called. If the output doesn't end
     * with '\n' the rest is used at the next call of parseOutput().
     *
     * @param output The output to split into lines and parse.
     */
    void parseOutput(const QString& output);

private:

    /**
     * Implement this method to parse a single line of the output.
     *
     * @param line The line to parse.
     */
    virtual void parseLine(const QString& line) = 0;

    QString restOutput;
};


} // namespace Cervisia


#endif // CERVISIA_OUTPUT_PARSER_H
