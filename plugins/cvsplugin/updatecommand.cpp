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

#include "updatecommand.h"
using Cervisia::UpdateCommand;

#include <dcopref.h>
#include <klocale.h>

#include <cvsservice_stub.h>
#include <update_parser.h>

#include "cvsplugin.h"

#include <kdebug.h>


UpdateCommand::UpdateCommand(const QStringList& files, UpdateParser* parser)
    : CvsCommandBase(Update)
    , m_fileList(files)
    , m_parser(parser)
    , m_simulation(false)
{
}


UpdateCommand::~UpdateCommand()
{
}


void UpdateCommand::setSimulation(bool simulation)
{
    m_simulation = simulation;
}


bool UpdateCommand::prepare()
{
    DCOPRef jobRef;

    if( m_simulation )
        jobRef = CvsPlugin::cvsService()->simulateUpdate(m_fileList, isRecursive(),
                        false/*opt_createDirs*/, false/*opt_pruneDirs*/);
    else
        jobRef = CvsPlugin::cvsService()->update(m_fileList, isRecursive(),
                        false/*opt_createDirs*/, false/*opt_pruneDirs*/, "");
    connectToJob(jobRef);

    kdDebug(8050) << "UpdateCommand::prepare(): connect parser to our signal" << endl;
    connect(this, SIGNAL(receivedStdout(const QString&)),
            m_parser, SLOT(parseOutput(const QString&)));

    return true;
}
