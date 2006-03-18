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

#ifndef CERVISIA_SVNCOMMANDBASE_H
#define CERVISIA_SVNCOMMANDBASE_H

#include <dcopobject.h>
#include <commandbase.h>

class DCOPRef;
class SvnJob_stub;
class SvnService_stub;


namespace Cervisia
{


class SvnCommandBase : public CommandBase, public DCOPObject
{
    K_DCOP
    Q_OBJECT

public:
    enum DeletionHandling { ManualDeletion, AutomaticDeletion };

    SvnCommandBase(const ActionKind& action);
    virtual ~SvnCommandBase();

    QString commandString() const;

    virtual bool isRunning() const;
    virtual void cancel();

    virtual bool prepare() = 0;
    virtual void execute();

    virtual bool isErrorMessage(const QString& /* line */) const { return false; }

k_dcop:
    void dcopJobExited(bool normalExit, int exitStatus);
    void dcopReceivedStdout(QString buffer);
    void dcopReceivedStderr(QString buffer);

protected:
    void connectToJob(const DCOPRef& jobRef,
                      DeletionHandling deletion = AutomaticDeletion);

    QString      m_errorId1, m_errorId2;
    bool         m_errorOccurred;

private:
    void processOutput(const QString& buffer);

    SvnJob_stub*     m_svnJob;
    QString          m_lineBuffer;
    DeletionHandling m_deletion;
};


}


#endif