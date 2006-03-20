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

#include "checkoutcommand.h"
using Cervisia::CheckoutCommand;

#include <dcopref.h>

#include <svnservice_stub.h>

#include "svnplugin.h"


CheckoutCommand::CheckoutCommand(const QString& workingFolder, const QString& repository)
    : SvnCommandBase(Checkout)
    , m_workingFolder(workingFolder)
    , m_repository(repository)
    , m_revision("")
{
}


CheckoutCommand::~CheckoutCommand()
{
}


void CheckoutCommand::setRevision(const QString& revision)
{
    m_revision = revision;
}


bool CheckoutCommand::prepare()
{
    DCOPRef jobRef = SvnPlugin::svnService()->checkout(m_repository, m_revision,
                                                       m_workingFolder, isRecursive());
    connectToJob(jobRef);

    return true;
}
