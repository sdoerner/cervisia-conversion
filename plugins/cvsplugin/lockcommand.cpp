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

#include "lockcommand.h"
using Cervisia::LockCommand;

#include <dcopref.h>
#include <klocale.h>
#include <cvsservice_stub.h>

#include "cvsplugin.h"


LockCommand::LockCommand(const QStringList& files, const ActionKind& action)
    : CvsCommandBase(action)
    , m_fileList(files)
{
}


LockCommand::~LockCommand()
{
}


bool LockCommand::prepare()
{
    DCOPRef jobRef;

    if( action() == Lock )
        jobRef = CvsPlugin::cvsService()->lock(m_fileList);
    else
        jobRef = CvsPlugin::cvsService()->unlock(m_fileList);

    connectToJob(jobRef);

    connect(this, SIGNAL(jobExited(bool, int)),
            this, SLOT(deleteLater()));

    return true;
}
