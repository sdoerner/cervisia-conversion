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

#include "cvsjob.h"
using Cervisia::CvsJob;

#include <dcopref.h>
#include <kdebug.h>

#include <cvsjob_stub.h>


CvsJob::CvsJob(const DCOPRef& jobRef, const ActionKind& action)
    : PluginJobBase(jobRef, action)
    , m_cvsJob(0)
{
    // create a DCOP stub for the cvs job
    m_cvsJob = new CvsJob_stub(jobRef);

/*    // establish connections to the signals of the cvs job
    connectDCOPSignal(jobRef.app(), jobRef.obj(), "jobExited(bool, int)",
                                "dcopJobExited(bool, int)", true);
    connectDCOPSignal(jobRef.app(), jobRef.obj(), "receivedStdout(QString)",
                                "dcopReceivedStdout(QString)", true);
    connectDCOPSignal(jobRef.app(), jobRef.obj(), "receivedStderr(QString)",
                                "dcopReceivedStderr(QString)", true);*/
}


CvsJob::~CvsJob()
{
    delete m_cvsJob;
}


QString CvsJob::commandString() const
{
    return m_cvsJob->cvsCommand();
}

#include "cvsjob.moc"
