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

#ifndef CERVISIA_UPDATECOMMAND_H
#define CERVISIA_UPDATECOMMAND_H

#include <svncommandbase.h>
#include <qstringlist.h>


namespace Cervisia
{

class UpdateParser;


class UpdateCommand : public SvnCommandBase
{
public:
    UpdateCommand(const QStringList& files, UpdateParser* parser);
    ~UpdateCommand();

    void setSimulation(bool simulation);

    virtual bool prepare();

private:
    QStringList   m_fileList;
    UpdateParser* m_parser;
    bool          m_simulation;
};


}


#endif
