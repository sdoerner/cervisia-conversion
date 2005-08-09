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


#ifndef CERVISIA_ANNOTATE_INFO_H
#define CERVISIA_ANNOTATE_INFO_H


#include <qstring.h>


namespace Cervisia
{


/**
 * Dumb data struct to store the results of the annotate command. An instance
 * of this struct represents an annoteted line of the file.
 *
 * If you want additional informations use also LogInfo.
 */
struct AnnotateInfo
{
    /**
     * The revision the line was changed.
     */
    QString m_revision;

    /**
     * The content of the line.
     */
    QString m_line;
};


typedef QValueList<AnnotateInfo> AnnotateInfoList;

} // namespace Cervisia


#endif // CERVISIA_ANNOTATE_INFO_H
