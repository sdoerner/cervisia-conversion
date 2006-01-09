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

#include "updatetagcommand.h"
using Cervisia::UpdateTagCommand;

#include <kprocess.h>

#include <qapplication.h>

#include <updatedlg.h>

#include "cvsplugin.h"


UpdateTagCommand::UpdateTagCommand(const QStringList& files, UpdateParser* parser)
    : UpdateCommand(files, parser)
{
}


UpdateTagCommand::~UpdateTagCommand()
{
}


bool UpdateTagCommand::prepare()
{
    UpdateDialog dlg(CvsPlugin::cvsService(), qApp->activeWindow());

    if( !dlg.exec() )
        return false;

    QString tagopt;
    if( dlg.byTag() )
    {
        tagopt = "-r ";
        tagopt += dlg.tag();
    }
    else
    {
        tagopt = "-D ";
        tagopt += KProcess::quote(dlg.date());
    }
    tagopt += " ";

    setExtraOption(tagopt);

    return UpdateCommand::prepare();
}
