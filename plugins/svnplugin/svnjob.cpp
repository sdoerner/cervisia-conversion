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

#include "svnjob.h"
using Cervisia::SvnJob;

#include <dcopref.h>
#include <kdebug.h>

#include <svnjob_stub.h>


SvnJob::SvnJob(const DCOPRef& jobRef, const ActionKind& action)
    : PluginJobBase(jobRef, action)
    , m_svnJob(0)
{
    // create a DCOP stub for the svn job
    m_svnJob = new SvnJob_stub(jobRef);

/*    // establish connections to the signals of the svn job
    connectDCOPSignal(jobRef.app(), jobRef.obj(), "jobExited(bool, int)",
                                "dcopJobExited(bool, int)", true);
    connectDCOPSignal(jobRef.app(), jobRef.obj(), "receivedStdout(QString)",
                                "dcopReceivedStdout(QString)", true);
    connectDCOPSignal(jobRef.app(), jobRef.obj(), "receivedStderr(QString)",
                                "dcopReceivedStderr(QString)", true);*/
}


SvnJob::~SvnJob()
{
    delete m_svnJob;
}


QString SvnJob::commandString() const
{
    return m_svnJob->command();
}

#include "svnjob.moc"
