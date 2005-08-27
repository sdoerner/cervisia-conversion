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

#include "commitcommand.h"
using Cervisia::CommitCommand;

#include <dcopref.h>
#include <klocale.h>
#include <commitdlg.h>
#include <cvsservice_stub.h>

#include "cvsplugin.h"


CommitCommand::CommitCommand(const QStringList& files)
    : CvsCommandBase(Commit)
    , m_fileList(files)
{
}


CommitCommand::~CommitCommand()
{
}


// TODO: feature commit finished notification
bool CommitCommand::prepare()
{
    // modal dialog
    CommitDialog dlg;
    dlg.setCaption(i18n("CVS Commit"));
//     dlg.setLogMessage(changelogstr);
//     dlg.setLogHistory(recentCommits);
    dlg.setFileList(m_fileList);

    // did the user cancel the dialog?
    if( !dlg.exec() )
        return false;

    // get new list of files
    m_fileList = dlg.fileList();
    if( m_fileList.isEmpty() )
        return;

    QString msg = dlg.logMessage();
//         if( !recentCommits.contains(msg) )
//         {
//             recentCommits.prepend(msg);
//             while( recentCommits.count() > 50 )
//                 recentCommits.remove(recentCommits.last());
// 
//             KConfig* conf = config();
//             conf->setGroup("CommitLogs");
//             conf->writeEntry(sandbox, recentCommits, COMMIT_SPLIT_CHAR);
//         }

    DCOPRef jobRef = CvsPlugin::cvsService()->commit(m_fileList, msg, isRecursive());
    connectToJob(jobRef);

    return true;
}
