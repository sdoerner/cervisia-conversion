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

#include <loginfo.h>
#include <annotatedlg.h>
#include <progressdlg.h>
#include <svnservice_stub.h>
#include <cervisiasettings.h>

#include "logcommand.h"
#include "svnplugin.h"
#include "svn_annotate_parser.h"


AnnotateCommand::AnnotateCommand(const QString& fileName, const QString& revision)
    : SvnCommandBase(Other)
    , m_fileName(fileName)
    , m_revision(revision)
    , m_annotateParser(new SvnAnnotateParser())
    , m_annotateDlg(0)
    , m_logCmd(new Cervisia::LogCommand(fileName))
    , m_logDone(false)
    , m_annotateDone(false)
{
    m_logCmd->setBatchMode(true);
}


AnnotateCommand::~AnnotateCommand()
{
}


bool AnnotateCommand::prepare()
{
    connect(m_logCmd, SIGNAL(jobExited(bool, int)),
            this, SLOT(logProcessExited()));

    if( m_logCmd->prepare() )
        m_logCmd->execute();

    DCOPRef jobRef = SvnPlugin::svnService()->annotate(m_fileName, m_revision);
    connectToJob(jobRef);

    connect(this, SIGNAL(receivedStdout(const QString&)),
            m_annotateParser, SLOT(parseOutput(const QString&)));

    connect(this, SIGNAL(jobExited(bool, int)),
            this, SLOT(annotateProcessExited()));

    KConfig* partConfig = CervisiaSettings::self()->config();
    m_annotateDlg = new AnnotateDialog(*partConfig);

    return true;
}


void AnnotateCommand::execute()
{
    ProgressDialog* dlg = new ProgressDialog(m_annotateDlg, "Annotate", i18n("SVN Annotate"));
    dlg->execute(this);

    SvnCommandBase::execute();
}


void AnnotateCommand::logProcessExited()
{
    m_logDone = true;

    m_logInfos = m_logCmd->logInfos();

    if( m_logDone && m_annotateDone )
        showDialog();
}


void AnnotateCommand::annotateProcessExited()
{
    m_annotateDone = true;

    if( m_logDone && m_annotateDone )
        showDialog();
}


void AnnotateCommand::showDialog()
{
    kdDebug(8050) << "AnnotateCommand::showDialog()" << endl;

    // error occurred or process was canceled
    if( m_errorOccurred )
        return;

    m_annotateDlg->setCaption(i18n("SVN Annotate: %1").arg(m_fileName));
    m_annotateDlg->setAnnotateInfos(m_logInfos,
                                    m_annotateParser->annotateInfos());
    m_annotateDlg->show();
}

#include "annotatecommand.moc"
