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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "annotatecommand.h"
using Cervisia::AnnotateCommand;

#include <dcopref.h>
#include <klocale.h>

#include <qapplication.h>

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
    , m_annotateParser(new CvsAnnotateParser(this))
    , m_logParser(new CvsLogParser(this))
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
    connectToJob(jobRef, ManualDeletion);

    connect(this, SIGNAL(receivedStdout(const QString&)),
            m_annotateParser, SLOT(parseOutput(const QString&)));
    connect(this, SIGNAL(receivedStdout(const QString&)),
            m_logParser, SLOT(parseOutput(const QString&)));

    connect(this, SIGNAL(jobExited(bool, int)),
            this, SLOT(showDialog()));

    KConfig* partConfig = CervisiaSettings::self()->config();
    m_annotateDlg = new AnnotateDialog(*partConfig, qApp->activeWindow());

    return true;
}


void AnnotateCommand::execute()
{
    ProgressDialog* dlg = new ProgressDialog(m_annotateDlg, "Annotate", i18n("CVS Annotate"));
    dlg->execute(this);

    CvsCommandBase::execute();
}


void AnnotateCommand::showDialog()
{
    // error occurred or process was canceled
    if( m_errorOccurred )
        return;

    m_annotateDlg->setCaption(i18n("CVS Annotate: %1").arg(m_fileName));
    m_annotateDlg->setAnnotateInfos(m_logParser->logInfos(),
                                    m_annotateParser->annotateInfos());
    m_annotateDlg->show();

    deleteLater();
}

#include "annotatecommand.moc"
