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

#ifndef CERVISIA_PLUGINJOBBASE_H
#define CERVISIA_PLUGINJOBBASE_H

#include <qobject.h>
#include <dcopobject.h>

class DCOPRef;


namespace Cervisia
{


class PluginJobBase : public QObject, public DCOPObject
{
    K_DCOP
    Q_OBJECT

public:
    enum ActionKind { Add, Remove, Update, SimulateUpdate, Commit, Other };

    PluginJobBase(const DCOPRef& jobRef, const ActionKind& action);
    ~PluginJobBase();

    virtual QString commandString() const = 0;
    ActionKind action() const { return m_action; }

    bool isRecursive() const { return m_recursive; }
    void setRecursive(bool recursive) { m_recursive = recursive; }

k_dcop:
    void dcopJobExited(bool normalExit, int exitStatus);
    void dcopReceivedStdout(QString buffer);
    void dcopReceivedStderr(QString buffer);

signals:
    void jobExited(bool normalExit, int status);
    void receivedStdout(const QString& buffer);
    void receivedStderr(const QString& buffer);
    void receivedLine(const QString& line);

private:
    void processOutput(const QString& buffer);

    ActionKind m_action;
    bool       m_recursive;
    QString    m_lineBuffer;
};


}


#endif
