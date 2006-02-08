/*
 * Copyright (c) 2006 Christian Loose <christian.loose@kdemail.net>
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

#include "fetchmodulelistcommand.h"
using Cervisia::FetchModuleListCommand;

#include <dcopref.h>
#include <klocale.h>

#include <progressdlg.h>
#include <cvsservice_stub.h>

#include "cvsplugin.h"


FetchModuleListCommand::FetchModuleListCommand(const QString& repository)
    : CvsCommandBase(Other)
    , m_repository(repository)
    , m_progressDlg(0)
{
    m_errorId1 = "cvs checkout:";
    m_errorId2 = "cvs [checkout aborted]:";
}


FetchModuleListCommand::~FetchModuleListCommand()
{
    delete m_progressDlg;
}


bool FetchModuleListCommand::prepare()
{
    DCOPRef jobRef = CvsPlugin::cvsService()->moduleList(m_repository);
    connectToJob(jobRef);

    connect(this, SIGNAL(receivedLine(const QString&)),
            this, SLOT(parseLine(const QString&)));

    return true;
}


void FetchModuleListCommand::execute()
{
    m_progressDlg = new ProgressDialog(0, "Checkout", i18n("CVS Checkout"));
    m_progressDlg->execute(this);

    CvsCommandBase::execute();
}


QStringList FetchModuleListCommand::moduleList()
{
    return m_moduleList;
}


void FetchModuleListCommand::parseLine(const QString& line)
{
    if( line.startsWith("Unknown host") )
        return;

    int pos = line.find(' ');
    if( pos == -1 )
        pos = line.find('\t');
    if( pos == -1 )
        pos = line.length();

    QString module = line.left(pos).stripWhiteSpace();
    if( !module.isEmpty() )
        m_moduleList.push_back(module);
}

#include "fetchmodulelistcommand.moc"
