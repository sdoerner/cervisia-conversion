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

#ifndef CERVISIA_CREATEPATCHCOMMAND_H
#define CERVISIA_CREATEPATCHCOMMAND_H

#include <cvscommandbase.h>

#include <qstringlist.h>


namespace Cervisia
{


class CreatePatchCommand : public CvsCommandBase
{
    Q_OBJECT

public:
    CreatePatchCommand(const QString& fileName, const QString& revisionA,
                       const QString& revisionB);
    ~CreatePatchCommand();

    virtual bool prepare();
    virtual void execute();

private slots:
    void receivedLine(const QString& line);
    void saveToFile();

private:
    QString     m_fileName;
    QString     m_revisionA;
    QString     m_revisionB;
    QStringList m_processOutput;
};


}


#endif
