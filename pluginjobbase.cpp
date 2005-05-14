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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "pluginjobbase.h"
using Cervisia::PluginJobBase;

#include <dcopref.h>
#include <kdebug.h>


PluginJobBase::PluginJobBase(const DCOPRef& jobRef, const ActionKind& action)
    : DCOPObject(jobRef.obj())
    , QObject(0, jobRef.obj())
    , m_action(action)
{
    // establish connections to the signals of the job
    connectDCOPSignal(jobRef.app(), jobRef.obj(), "jobExited(bool, int)",
                      "dcopJobExited(bool, int)", true);
    connectDCOPSignal(jobRef.app(), jobRef.obj(), "receivedStdout(QString)",
                      "dcopReceivedStdout(QString)", true);
    connectDCOPSignal(jobRef.app(), jobRef.obj(), "receivedStderr(QString)",
                      "dcopReceivedStderr(QString)", true);
}


PluginJobBase::~PluginJobBase()
{
}


void PluginJobBase::dcopJobExited(bool normalExit, int exitStatus)
{
    kdDebug(8050) << "PluginJobBase::dcopJobExited(): normalExit = " << normalExit << endl;
    emit jobExited(normalExit, exitStatus);
    deleteLater();  //TODO: Okay?
}


void PluginJobBase::dcopReceivedStdout(QString buffer)
{
    kdDebug(8050) << "PluginJobBase::dcopReceivedStdout(): buffer = " << buffer << endl;

    processOutput(buffer);

    emit receivedStdout(buffer);
}


void PluginJobBase::dcopReceivedStderr(QString buffer)
{
    kdDebug(8050) << "PluginJobBase::dcopReceivedStderr(): buffer = " << buffer << endl;

    processOutput(buffer);

    emit receivedStderr(buffer);
}


void PluginJobBase::processOutput(const QString& buffer)
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

#include "pluginjobbase.moc"
