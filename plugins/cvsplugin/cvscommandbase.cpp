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

#include "cvscommandbase.h"
using Cervisia::CvsCommandBase;

#include <dcopref.h>
#include <cvsjob_stub.h>

#include <kdebug.h>


CvsCommandBase::CvsCommandBase(const ActionKind& action)
    : CommandBase(action)
    , m_errorOccurred(false)
    , m_cvsJob(0)
{
//     kdDebug(8050) << k_funcinfo << endl;
}


CvsCommandBase::~CvsCommandBase()
{
    delete m_cvsJob;
}


QString CvsCommandBase::commandString() const
{
    return m_cvsJob->cvsCommand();
}


bool CvsCommandBase::isRunning() const
{
    return m_cvsJob->isRunning();
}


void CvsCommandBase::cancel()
{
    kdDebug(8050) << k_funcinfo << endl;
    m_cvsJob->cancel();
}


void CvsCommandBase::execute()
{
    kdDebug(8050) << k_funcinfo << endl;
    m_cvsJob->execute();
}


bool CvsCommandBase::isErrorMessage(const QString& line) const
{
    return ( line.startsWith(m_errorId1) ||
             line.startsWith(m_errorId2) ||
             line.startsWith("cvs [server aborted]:") );
}


void CvsCommandBase::dcopJobExited(bool normalExit, int exitStatus)
{
    kdDebug(8050) << k_funcinfo << "normalExit = " << normalExit << endl;

    // do we have some output left to process
    if( !m_lineBuffer.isEmpty() )
    {
        processOutput("\n");
    }

    emit jobExited(normalExit, exitStatus);
}


void CvsCommandBase::dcopReceivedStdout(QString buffer)
{
    kdDebug(8050) << k_funcinfo << "buffer = " << buffer << endl;

    processOutput(buffer);
    emit receivedStdout(buffer);
}


void CvsCommandBase::dcopReceivedStderr(QString buffer)
{
    kdDebug(8050) << k_funcinfo << "buffer = " << buffer << endl;

    processOutput(buffer);
    emit receivedStderr(buffer);
}


void CvsCommandBase::connectToJob(const DCOPRef& jobRef, DeletionHandling deletion)
{
    m_cvsJob = new CvsJob_stub(jobRef);

    // establish connections to the signals of the job
    connectDCOPSignal(jobRef.app(), jobRef.obj(), "jobExited(bool, int)",
                      "dcopJobExited(bool, int)", true);
    connectDCOPSignal(jobRef.app(), jobRef.obj(), "receivedStdout(QString)",
                      "dcopReceivedStdout(QString)", true);
    connectDCOPSignal(jobRef.app(), jobRef.obj(), "receivedStderr(QString)",
                      "dcopReceivedStderr(QString)", true);

    if( deletion == AutomaticDeletion )
    {
        connect(this, SIGNAL(jobExited(bool, int)),
                this, SLOT(deleteLater()));
    }
}


void CvsCommandBase::processOutput(const QString& buffer)
{
    m_lineBuffer += buffer;

    int pos;
    while( (pos = m_lineBuffer.find('\n')) != -1 )
    {
        QString line = m_lineBuffer.left(pos);
        if( !line.isEmpty() )
        {
            // check for error messages
            if( isErrorMessage(line) )
                m_errorOccurred = true;

            emit receivedLine(line);
        }

        m_lineBuffer = m_lineBuffer.right(m_lineBuffer.length()-pos-1);
    }
}

#include "cvscommandbase.moc"
