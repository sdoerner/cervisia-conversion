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

#include "addcommand.h"
using Cervisia::AddCommand;

#include <dcopref.h>
#include <addremovedlg.h>
#include <cvsservice_stub.h>

#include "cvsplugin.h"


AddCommand::AddCommand(const QStringList& files)
    : CvsCommandBase(Add)
    , m_fileList(files)
    , m_binary(false)
{
}


AddCommand::~AddCommand()
{
}


void AddCommand::setBinary(bool binary)
{
    m_binary = binary;
}


bool AddCommand::prepare()
{
    AddRemoveDialog::ActionType dialogType = m_binary ? AddRemoveDialog::AddBinary
                                                      : AddRemoveDialog::Add;

    // modal dialog
    AddRemoveDialog dlg(dialogType);
    dlg.setFileList(m_fileList);

    if( dlg.exec() )
    {
        DCOPRef jobRef = CvsPlugin::cvsService()->add(m_fileList, m_binary);
        connectToJob(jobRef);

        return true;
    }

    return false;
}
