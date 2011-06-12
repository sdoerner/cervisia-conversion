/*
 * Copyright (c) 2003 Christian Loose <christian.loose@hamburg.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#ifndef SSHAGENT_H
#define SSHAGENT_H

#include <tqobject.h>
#include <tqstring.h>
#include <tqstringlist.h>

class KProcess;


class SshAgent : public TQObject
{
    Q_OBJECT
  TQ_OBJECT

public:
    SshAgent(TQObject* tqparent = 0, const char* name = 0);
    ~SshAgent();

    bool querySshAgent();
    bool addSshIdentities();
    void killSshAgent();

    bool isRunning() const { return m_isRunning; }
    TQString pid() const { return m_pid; }
    TQString authSock() const { return m_authSock; }

private slots:
    void slotProcessExited(KProcess*);
    void slotReceivedStdout(KProcess* proc, char* buffer, int buflen);
    void slotReceivedStderr(KProcess* proc, char* buffer, int buflen);

private:
    bool startSshAgent();

    TQStringList    m_outputLines;

    static bool    m_isRunning;
    static bool    m_isOurAgent;
    static TQString m_authSock;
    static TQString m_pid;
};


#endif
