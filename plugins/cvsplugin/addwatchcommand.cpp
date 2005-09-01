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

#include "addwatchcommand.h"
using Cervisia::AddWatchCommand;

#include <dcopref.h>
#include <watchdlg.h>
#include <cvsservice_stub.h>

#include "cvsplugin.h"


AddWatchCommand::AddWatchCommand(const QStringList& files)
    : CvsCommandBase(Add)
    , m_fileList(files)
{
}


AddWatchCommand::~AddWatchCommand()
{
}


bool AddWatchCommand::prepare()
{
    // modal dialog
    WatchDialog dlg(WatchDialog::Add);

    if( dlg.exec() && dlg.events() != WatchDialog::None )
    {
        DCOPRef jobRef = CvsPlugin::cvsService()->addWatch(m_fileList, dlg.events());
        connectToJob(jobRef);

        connect(this, SIGNAL(jobExited(bool, int)),
                this, SLOT(deleteLater()));

        return true;
    }

    return false;
}
