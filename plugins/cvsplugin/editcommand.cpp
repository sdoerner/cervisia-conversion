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

#include "editcommand.h"
using Cervisia::EditCommand;

#include <dcopref.h>
#include <klocale.h>
#include <cvsservice_stub.h>

#include "cvsplugin.h"


EditCommand::EditCommand(const QStringList& files, const ActionKind& action)
    : CvsCommandBase(action)
    , m_fileList(files)
{
}


EditCommand::~EditCommand()
{
}


bool EditCommand::prepare()
{
    DCOPRef jobRef;

    if( action() == Edit )
        jobRef = CvsPlugin::cvsService()->edit(m_fileList);
    else
        jobRef = CvsPlugin::cvsService()->unedit(m_fileList);

    connectToJob(jobRef);

    return true;
}
