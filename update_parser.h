/*
 * Copyright (c) 2005 Christian Loose <christian.loose@kdemail.net>
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
 * Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef CERVISIA_UPDATE_PARSER_H
#define CERVISIA_UPDATE_PARSER_H

#include "output_parser.h"
#include "entry.h"


namespace Cervisia
{


class UpdateParser : public OutputParser
{
    Q_OBJECT

public:
    UpdateParser();
    virtual ~UpdateParser();

    bool isSimulation() const;
    void setSimulation(bool simulation);

signals:
    void updateItemStatus(const QString& filePath, Cervisia::EntryStatus status, bool isdir);

private:
    bool m_simulation;
};


}


#endif
