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

#include "diffcommand.h"
using Cervisia::DiffCommand;

#include <dcopref.h>
#include <kdebug.h>
#include <klocale.h>

#include <qapplication.h>

#include <cervisiasettings.h>
#include <diff_parser.h>
#include <diffdlg.h>
#include <progressdlg.h>
#include <cvsservice_stub.h>

#include "cvsplugin.h"


DiffCommand::DiffCommand(const QString& fileName,
                         const QString& revisionA,
                         const QString& revisionB,
                         const QStringList& options)
    : CvsCommandBase(Other)
    , m_fileName(fileName)
    , m_revisionA(revisionA)
    , m_revisionB(revisionB)
    , m_options(options)
    , m_parser(new DiffParser(this))
    , m_diffDialog(0)
{
    m_errorId1 = "cvs diff:";
    m_errorId2 = "cvs [diff aborted]:";
}


DiffCommand::~DiffCommand()
{
}


const Cervisia::DiffInfoList& DiffCommand::diffInfos() const
{
    return m_parser->diffInfos();
}


bool DiffCommand::prepare()
{
    DCOPRef jobRef = CvsPlugin::cvsService()->diff(m_fileName,
                                                   m_revisionA,
                                                   m_revisionB,
                                                   m_options);
    connectToJob(jobRef, ManualDeletion);

    connect(this, SIGNAL(receivedStdout(const QString&)),
            m_parser, SLOT(parseOutput(const QString&)));

    connect(this, SIGNAL(jobExited(bool, int)),
            this, SLOT(showDialog()));

    KConfig* partConfig = CervisiaSettings::self()->config();
    m_diffDialog = new DiffDialog(*partConfig, qApp->activeWindow());

    return true;
}


void DiffCommand::execute()
{
    ProgressDialog* dlg = new ProgressDialog(m_diffDialog, "Diffing", i18n("CVS Diff"));
    dlg->execute(this);

    CvsCommandBase::execute();
}


void DiffCommand::showDialog()
{
    // error occurred or process was canceled
    if( m_errorOccurred )
        return;

    m_diffDialog->setCaption(i18n("CVS Diff: %1").arg(m_fileName));
    m_diffDialog->setDiffInfos(m_parser->diffInfos());
    m_diffDialog->setRevisionA(m_revisionA);
    m_diffDialog->setRevisionB(m_revisionB);
    m_diffDialog->show();

    deleteLater();
}


#include "diffcommand.moc"
