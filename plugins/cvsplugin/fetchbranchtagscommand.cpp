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

#include "fetchbranchtagscommand.h"
using Cervisia::FetchBranchTagsCommand;

#include <dcopref.h>
#include <klocale.h>

#include <progressdlg.h>
#include <cvsservice_stub.h>

#include "cvsplugin.h"


FetchBranchTagsCommand::FetchBranchTagsCommand(const QString& repository, const QString& module)
    : CvsCommandBase(Other)
    , m_repository(repository)
    , m_module(module)
    , m_progressDlg(0)
{
    m_errorId1 = "cvs rlog:";
    m_errorId2 = "cvs [rlog aborted]:";
}


FetchBranchTagsCommand::~FetchBranchTagsCommand()
{
//     delete m_progressDlg;
}


bool FetchBranchTagsCommand::prepare()
{
    DCOPRef jobRef = CvsPlugin::cvsService()->rlog(m_repository, m_module,
                                                   isRecursive());
    connectToJob(jobRef);

    connect(this, SIGNAL(receivedLine(const QString&)),
            this, SLOT(parseLine(const QString&)));

    return true;
}


void FetchBranchTagsCommand::execute()
{
    m_progressDlg = new ProgressDialog(0, "Remote Logging", i18n("CVS RLog"));
    m_progressDlg->execute(this);

    CvsCommandBase::execute();
}


QStringList FetchBranchTagsCommand::branchTagList()
{
    return m_branchTagList;
}


void FetchBranchTagsCommand::parseLine(const QString& line)
{
    int colonPos;

    if( line.isEmpty() || line[0] != '\t' )
        return;

    if( (colonPos = line.find(':', 1)) < 0 )
        return;

    const QString tag  = line.mid(1, colonPos - 1);
    if( !m_branchTagList.contains(tag) )
        m_branchTagList.push_back(tag);
}

#include "fetchbranchtagscommand.moc"
