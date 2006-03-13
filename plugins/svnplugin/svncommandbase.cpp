/*
 * Copyright (c) 2005-2006 Christian Loose <christian.loose@kdemail.net>
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

#include "svncommandbase.h"
using Cervisia::SvnCommandBase;

#include <dcopref.h>
#include <svnjob_stub.h>

#include <kdebug.h>


SvnCommandBase::SvnCommandBase(const ActionKind& action)
    : CommandBase(action)
    , m_errorOccurred(false)
    , m_svnJob(0)
    , m_deletion(AutomaticDeletion)
{
//     kdDebug(8050) << k_funcinfo << endl;
}


SvnCommandBase::~SvnCommandBase()
{
    delete m_svnJob;
}


bool SvnCommandBase::isRunning() const
{
    return m_svnJob->isRunning();
}


QString SvnCommandBase::commandString() const
{
    return m_svnJob->command();
}


void SvnCommandBase::cancel()
{
    m_svnJob->cancel();
}


void SvnCommandBase::execute()
{
    kdDebug(8050) << k_funcinfo << endl;
    m_svnJob->execute();
}


void SvnCommandBase::dcopJobExited(bool normalExit, int exitStatus)
{
    kdDebug(8050) << k_funcinfo << "normalExit = " << normalExit << endl;

    // do we have some output left to process
    if( !m_lineBuffer.isEmpty() )
    {
        processOutput("\n");
    }

    emit jobExited(normalExit, exitStatus);

    if( m_deletion == AutomaticDeletion )
        deleteLater();
}


void SvnCommandBase::dcopReceivedStdout(QString buffer)
{
//     kdDebug(8050) << k_funcinfo << "buffer = " << buffer << endl;
    kdDebug(8050) << k_funcinfo << endl;

    processOutput(buffer);
    emit receivedStdout(buffer);
}


void SvnCommandBase::dcopReceivedStderr(QString buffer)
{
//     kdDebug(8050) << k_funcinfo << "buffer = " << buffer << endl;
    kdDebug(8050) << k_funcinfo << endl;

    processOutput(buffer);
    emit receivedStderr(buffer);
}


void SvnCommandBase::connectToJob(const DCOPRef& jobRef, DeletionHandling deletion)
{
    m_svnJob = new SvnJob_stub(jobRef);

    // establish connections to the signals of the job
    connectDCOPSignal(jobRef.app(), jobRef.obj(), "jobExited(bool, int)",
                      "dcopJobExited(bool, int)", true);
    connectDCOPSignal(jobRef.app(), jobRef.obj(), "receivedStdout(QString)",
                      "dcopReceivedStdout(QString)", true);
    connectDCOPSignal(jobRef.app(), jobRef.obj(), "receivedStderr(QString)",
                      "dcopReceivedStderr(QString)", true);
}


void SvnCommandBase::processOutput(const QString& buffer)
{
    m_lineBuffer += buffer;

    int pos;
    while( (pos = m_lineBuffer.find('\n')) != -1 )
    {
        QString line = m_lineBuffer.left(pos);
        if( !line.isEmpty() )
            emit receivedLine(line);

        m_lineBuffer = m_lineBuffer.right(m_lineBuffer.length()-pos-1);
    }
}

#include "svncommandbase.moc"
