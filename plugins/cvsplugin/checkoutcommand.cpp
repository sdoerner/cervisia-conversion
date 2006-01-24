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

#include <qapplication.h>

#include <cvsservice_stub.h>

#include "cvsplugin.h"

#include <kdebug.h>

CheckoutCommand::CheckoutCommand(const QString& workingFolder, const QString& repository,
                                 const QString& module)
    : CvsCommandBase(Checkout)
    , m_workingFolder(workingFolder)
    , m_repository(repository)
    , m_module(module)
    , m_branchTag("")
    , m_aliasName("")
    , m_pruneDirectories(false)
    , m_exportOnly(false)
{
}


CheckoutCommand::~CheckoutCommand()
{
}


void CheckoutCommand::setBranchTag(const QString& tag)
{
    m_branchTag = tag;
}


void CheckoutCommand::setAliasName(const QString& alias)
{
    m_aliasName = alias;
}


void CheckoutCommand::setPruneDirectories(bool pruneDirs)
{
    m_pruneDirectories = pruneDirs;
}


void CheckoutCommand::setExportOnly(bool exportOnly)
{
    m_exportOnly = exportOnly;
}


bool CheckoutCommand::prepare()
{
    kdDebug(8050) << k_funcinfo << endl;

    DCOPRef jobRef = CvsPlugin::cvsService()->checkout(m_workingFolder, m_repository,
                                                       m_module, m_branchTag,
                                                       m_pruneDirectories, m_aliasName,
                                                       m_exportOnly, isRecursive());
    connectToJob(jobRef);

    return true;
}
