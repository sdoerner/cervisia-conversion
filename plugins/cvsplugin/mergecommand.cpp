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

#include "mergecommand.h"
using Cervisia::MergeCommand;

#include <dcopref.h>
#include <klocale.h>

#include <qapplication.h>

#include <mergedlg.h>
#include <cvsservice_stub.h>

#include "cvsplugin.h"


MergeCommand::MergeCommand(const QStringList& files)
    : CvsCommandBase(Update)
    , m_fileList(files)
    , m_createDirectories(false)
    , m_pruneDirectories(false)
{
}


MergeCommand::~MergeCommand()
{
}


void MergeCommand::setCreateDirectories(bool createDirs)
{
    m_createDirectories = createDirs;
}


void MergeCommand::setPruneDirectories(bool pruneDirs)
{
    m_pruneDirectories = pruneDirs;
}


bool MergeCommand::prepare()
{
    // modal dialog
    MergeDialog dlg(CvsPlugin::cvsService(), qApp->activeWindow());

    if( dlg.exec() )
    {
        QString tagOption;
        if( dlg.byBranch() )
        {
            tagOption = "-j ";
            tagOption += dlg.branch();
        }
        else
        {
            tagOption = "-j ";
            tagOption += dlg.tag1();
            tagOption += " -j ";
            tagOption += dlg.tag2();
        }
        tagOption += " ";

        DCOPRef jobRef = CvsPlugin::cvsService()->update(m_fileList, isRecursive(),
                                m_createDirectories, m_pruneDirectories, tagOption);
        connectToJob(jobRef);

        return true;
    }

    return false;
}
