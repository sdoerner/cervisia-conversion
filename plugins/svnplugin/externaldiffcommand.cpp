/*
 * Copyright (c) 2006 André Wöbbeking <Woebbeking@web.de>
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

#include "externaldiffcommand.h"

#include "svnplugin.h"

#include <misc.h>
#include <progressdlg.h>
#include <svnservice_stub.h>

#include <dcopref.h>
#include <klocale.h>
#include <kprocess.h>

#include <qapplication.h>
#include <qfile.h>
#include <qfileinfo.h>


using Cervisia::ExternalDiffCommand;


ExternalDiffCommand::ExternalDiffCommand(const QString& diffApplication,
                                         const QString& fileName,
                                         const QString& revisionA,
                                         const QString& revisionB)
    : SvnCommandBase(Other)
    , m_diffApplication(diffApplication)
    , m_fileName(fileName)
    , m_revisionA(revisionA)
    , m_revisionB(revisionB)
{
    Q_ASSERT(!m_diffApplication.isEmpty());
    Q_ASSERT(!m_fileName.isEmpty());
    Q_ASSERT(!m_revisionA.isEmpty());
    Q_ASSERT(!m_revisionB.isEmpty());
}


ExternalDiffCommand::~ExternalDiffCommand()
{
}


bool ExternalDiffCommand::prepare()
{
    const QString suffixA('-' + m_revisionA + '-' + QFileInfo(m_fileName).fileName());
    const QString suffixB('-' + m_revisionB + '-' + QFileInfo(m_fileName).fileName());
    m_tempFileNameA = ::tempFileName(suffixA);
    m_tempFileNameB = ::tempFileName(suffixB);

    DCOPRef jobRef = SvnPlugin::svnService()->downloadRevision(m_fileName,
                                                               m_revisionA,
                                                               m_tempFileNameA,
                                                               m_revisionB,
                                                               m_tempFileNameB);
    connectToJob(jobRef);

    connect(this, SIGNAL(jobExited(bool, int)),
            this, SLOT(diff()));

    return true;
}


void ExternalDiffCommand::execute()
{
    ProgressDialog* dlg = new ProgressDialog(qApp->activeWindow(), i18n("Diffing"), i18n("SVN Diff"));
    dlg->execute(this);

    SvnCommandBase::execute();
}


void ExternalDiffCommand::diff()
{
    // error occurred or process was canceled
    if( m_errorOccurred )
        return;

    QString cmdline(m_diffApplication);
    cmdline += ' ';
    cmdline += KProcess::quote(m_tempFileNameA);
    cmdline += ' ';
    cmdline += KProcess::quote(m_tempFileNameB);

    KProcess proc;
    proc.setUseShell(true, "/bin/sh");
    proc << cmdline;
    proc.start(KProcess::DontCare);
}


#include "externaldiffcommand.moc"
