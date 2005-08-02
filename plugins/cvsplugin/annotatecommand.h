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

#include <cvscommandbase.h>

#include <qmap.h>
#include <loginfo.h>

class AnnotateDialog;


namespace Cervisia
{

class CvsAnnotateParser;
class CvsLogParser;


class AnnotateCommand : public CvsCommandBase
{
    Q_OBJECT

public:
    explicit AnnotateCommand(const QString& fileName, const QString& revision = QString::null);
    ~AnnotateCommand();

    virtual bool prepare();
    virtual void execute();

private slots:
    void showDialog();

private:
    void setupLogInfoMap();
    void addAnnotateDataToDialog();

    QString            m_fileName;
    QString            m_revision;
    CvsAnnotateParser* m_annotateParser;
    CvsLogParser*      m_logParser;
    AnnotateDialog*    m_annotateDlg;

    QMap<QString, Cervisia::LogInfo> m_logInfoMap;
};


}


#endif
