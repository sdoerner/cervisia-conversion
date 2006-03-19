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

#include "view_command.h"

#include "svnplugin.h"

#include <misc.h>
#include <progressdlg.h>
#include <svnservice_stub.h>

#include <dcopref.h>
#include <klocale.h>
#include <krun.h>
#include <kurl.h>

#include <qapplication.h>
#include <qfile.h>
#include <qfileinfo.h>


using Cervisia::ViewCommand;


ViewCommand::ViewCommand(const QString& fileName, const QString& revision)
    : SvnCommandBase(Other)
    , m_fileName(fileName)
    , m_revision(revision)
{
}


ViewCommand::~ViewCommand()
{
}


bool ViewCommand::prepare()
{
    // create a temporary file
    const QString suffix('-' + m_revision + '-' + QFileInfo(m_fileName).fileName());
    m_tempFileName = ::tempFileName(suffix);

    DCOPRef jobRef = SvnPlugin::svnService()->downloadRevision(m_fileName,
                                                               m_revision,
                                                               m_tempFileName);
    connectToJob(jobRef);

    connect(this, SIGNAL(jobExited(bool, int)),
            this, SLOT(view()));

    return true;
}


void ViewCommand::execute()
{
    ProgressDialog* dlg = new ProgressDialog(qApp->activeWindow(), i18n("View"), i18n("View File"));
    dlg->execute(this);

    SvnCommandBase::execute();
}


void ViewCommand::view()
{
    // error occurred or process was canceled
    if( m_errorOccurred )
        return;

    // make file read-only
    chmod(QFile::encodeName(m_tempFileName), 0400);

    // open file in preferred editor
    KURL url;
    url.setPath(m_tempFileName);
    KRun* run = new KRun(url, 0, true, false);
    run->setRunExecutables(false);
}


#include "view_command.moc"
