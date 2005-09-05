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

#include "removecommand.h"
using Cervisia::RemoveCommand;

#include <dcopref.h>
#include <klocale.h>

#include <qapplication.h>

#include <addremovedlg.h>
#include <svnservice_stub.h>

#include "svnplugin.h"


RemoveCommand::RemoveCommand(const QStringList& files)
    : SvnCommandBase(Remove)
    , m_fileList(files)
{
}


RemoveCommand::~RemoveCommand()
{
}


bool RemoveCommand::prepare()
{
    // modal dialog
    AddRemoveDialog dlg(AddRemoveDialog::Remove, qApp->activeWindow());
    dlg.setCaption(i18n("SVN Remove"));
    dlg.setFileList(m_fileList);

    if( dlg.exec() )
    {
        DCOPRef jobRef = SvnPlugin::svnService()->remove(m_fileList);
        connectToJob(jobRef);

        return true;
    }

    return false;
}
