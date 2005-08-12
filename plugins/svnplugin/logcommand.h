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

#ifndef CERVISIA_LOGCOMMAND_H
#define CERVISIA_LOGCOMMAND_H

#include <svncommandbase.h>

#include <loginfo.h>

class LogDialog;


namespace Cervisia
{

class SvnLogParser;


class LogCommand : public SvnCommandBase
{
    Q_OBJECT

public:
    LogCommand(const QString& fileName);
    ~LogCommand();

    void setBatchMode(bool batchMode);
    const LogInfoList& logInfos() const;

    virtual bool prepare();
    virtual void execute();

private slots:
    void showDialog();

private:
    QString       m_fileName;
    SvnLogParser* m_parser;
    LogDialog*    m_logDlg;
    bool          m_batchMode;
};


}


#endif
