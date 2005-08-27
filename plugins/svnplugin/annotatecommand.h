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

#ifndef CERVISIA_ANNOTATECOMMAND_H
#define CERVISIA_ANNOTATECOMMAND_H

#include <svncommandbase.h>

#include <loginfo.h>

class AnnotateDialog;


namespace Cervisia
{

class SvnAnnotateParser;
class LogCommand;


class AnnotateCommand : public SvnCommandBase
{
    Q_OBJECT

public:
    explicit AnnotateCommand(const QString& fileName, const QString& revision = QString::null);
    ~AnnotateCommand();

    virtual bool prepare();
    virtual void execute();

private slots:
    void logProcessExited();
    void annotateProcessExited();

private:
    void showDialog();

    QString            m_fileName;
    QString            m_revision;
    SvnAnnotateParser* m_annotateParser;
    AnnotateDialog*    m_annotateDlg;
    LogCommand*        m_logCmd;
    bool               m_logDone, m_annotateDone;
    LogInfoList        m_logInfos;
};


}


#endif