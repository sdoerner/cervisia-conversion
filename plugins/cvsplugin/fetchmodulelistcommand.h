/*
 * Copyright (c) 2006 Christian Loose <christian.loose@kdemail.net>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef CERVISIA_FETCHMODULELISTCOMMAND_H
#define CERVISIA_FETCHMODULELISTCOMMAND_H

#include <cvscommandbase.h>

#include <qstringlist.h>

class ProgressDialog;


namespace Cervisia
{

class CvsPlugin;


class FetchModuleListCommand : public CvsCommandBase
{
    Q_OBJECT

public:
    explicit FetchModuleListCommand(const QString& repository);
    ~FetchModuleListCommand();

    virtual bool prepare();
    virtual void execute();

    QStringList moduleList();

private slots:
    void parseLine(const QString& line);

private:
    QString         m_repository;
    QStringList     m_moduleList;
    ProgressDialog* m_progressDlg;
};


}


#endif
