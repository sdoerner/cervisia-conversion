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

#include "deletetagcommand.h"
using Cervisia::DeleteTagCommand;

#include <dcopref.h>

#include <qapplication.h>

#include <tagdlg.h>
#include <cvsservice_stub.h>

#include "cvsplugin.h"


DeleteTagCommand::DeleteTagCommand(const QStringList& files)
    : CvsCommandBase(Other)	//??
    , m_fileList(files)
{
}


DeleteTagCommand::~DeleteTagCommand()
{
}


bool DeleteTagCommand::prepare()
{
    TagDialog dlg(TagDialog::Delete, CvsPlugin::cvsService(), qApp->activeWindow());

    // did the user cancel the dialog?
    if( !dlg.exec() )
        return false;

    DCOPRef jobRef = CvsPlugin::cvsService()->deleteTag(m_fileList, dlg.tag(),
                                                        dlg.branchTag(),
                                                        dlg.forceTag());
    connectToJob(jobRef);

    return true;
}
