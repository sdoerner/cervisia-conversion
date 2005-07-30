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

#include "logcommand.h"
using Cervisia::LogCommand;

#include <dcopref.h>
#include <klocale.h>

#include <logdlg.h>
#include <progressdlg.h>
#include <svnservice_stub.h>
#include <cervisiasettings.h>

#include "svnplugin.h"
#include "svn_log_parser.h"


LogCommand::LogCommand(const QString& fileName)
    : SvnCommandBase(Other)
    , m_fileName(fileName)
    , m_parser(new SvnLogParser())
    , m_logDlg(0)
{
//     m_errorId1 = "cvs log:";
//     m_errorId2 = "cvs [log aborted]:";
}


LogCommand::~LogCommand()
{
}


bool LogCommand::prepare()
{
    DCOPRef jobRef = SvnPlugin::svnService()->log(m_fileName);
    connectToJob(jobRef);

    connect(this, SIGNAL(receivedStdout(const QString&)),
            m_parser, SLOT(parseOutput(const QString&)));

    connect(this, SIGNAL(jobExited(bool, int)),
            this, SLOT(showDialog()));

    KConfig* partConfig = CervisiaSettings::self()->config();

    m_logDlg = new LogDialog(*partConfig);

    return true;
}


void LogCommand::execute()
{
    ProgressDialog* dlg = new ProgressDialog(m_logDlg, "Logging", i18n("SVN Log"));
    dlg->execute(this);

    SvnCommandBase::execute();
}


void LogCommand::showDialog()
{
    // error occurred or process was canceled
    if( m_errorOccurred )
        return;

    m_logDlg->setCaption(i18n("SVN Log: %1").arg(m_fileName));
    m_logDlg->setLogInfos(m_parser->logInfos());
    m_logDlg->show();
}

#include "logcommand.moc"