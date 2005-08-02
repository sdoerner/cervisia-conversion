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

#include "annotatecommand.h"
using Cervisia::AnnotateCommand;

#include <dcopref.h>
#include <klocale.h>

#include <annotatedlg.h>
#include <progressdlg.h>
#include <cvsservice_stub.h>
#include <cervisiasettings.h>

#include "cvsplugin.h"
#include "cvs_annotate_parser.h"
#include "cvs_log_parser.h"


AnnotateCommand::AnnotateCommand(const QString& fileName, const QString& revision)
    : CvsCommandBase(Other)
    , m_fileName(fileName)
    , m_revision(revision)
    , m_annotateParser(new CvsAnnotateParser())
    , m_logParser(new CvsLogParser())
    , m_annotateDlg(0)
{
    m_errorId1 = "cvs annotate:";
    m_errorId2 = "cvs [annotate aborted]:";
}


AnnotateCommand::~AnnotateCommand()
{
}


bool AnnotateCommand::prepare()
{
    DCOPRef jobRef = CvsPlugin::cvsService()->annotate(m_fileName, m_revision);
    connectToJob(jobRef);

    connect(this, SIGNAL(receivedStdout(const QString&)),
            m_annotateParser, SLOT(parseOutput(const QString&)));
    connect(this, SIGNAL(receivedStdout(const QString&)),
            m_logParser, SLOT(parseOutput(const QString&)));

    connect(this, SIGNAL(jobExited(bool, int)),
            this, SLOT(showDialog()));

    KConfig* partConfig = CervisiaSettings::self()->config();
    m_annotateDlg = new AnnotateDialog(*partConfig);

    return true;
}


void AnnotateCommand::execute()
{
    ProgressDialog* dlg = new ProgressDialog(m_annotateDlg, "Annotate", i18n("CVS Annotate"));
    dlg->execute(this);

    CvsCommandBase::execute();
}


void AnnotateCommand::setupLogInfoMap()
{
    typedef QValueList<Cervisia::LogInfo> LogInfoList;

    LogInfoList logInfos = m_logParser->logInfos();

    LogInfoList::ConstIterator it  = logInfos.begin();
    LogInfoList::ConstIterator end = logInfos.end();
    for( ; it != end; ++it )
    {
        m_logInfoMap.insert((*it).m_revision, *it);
    }
}


void AnnotateCommand::addAnnotateDataToDialog()
{
    typedef QValueList<Cervisia::AnnotateInfo> AnnotateInfoList;

    AnnotateInfoList annotateInfos = m_annotateParser->annotateInfos();

    bool odd = false;
    QString oldRevision = "";

    AnnotateInfoList::ConstIterator it  = annotateInfos.begin();
    AnnotateInfoList::ConstIterator end = annotateInfos.end();
    for( ; it != end; ++it )
    {
        // retrieve log information for current revision
        LogInfo logInfo = m_logInfoMap[(*it).m_revision];

        // revision changed?
        if( logInfo.m_revision != oldRevision )
        {
            oldRevision = logInfo.m_revision;
            odd = !odd;
        }
        else
        {
            logInfo.m_author = QString::null;
            logInfo.m_revision = QString::null;
        }

        m_annotateDlg->addLine(logInfo, (*it).m_line, odd);
    }
}


void AnnotateCommand::showDialog()
{
    // error occurred or process was canceled
    if( m_errorOccurred )
        return;

    setupLogInfoMap();
    addAnnotateDataToDialog();

    m_annotateDlg->setCaption(i18n("CVS Annotate: %1").arg(m_fileName));
    m_annotateDlg->show();
}

#include "annotatecommand.moc"
