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

#ifndef CERVISIA_COMMANDBASE_H
#define CERVISIA_COMMANDBASE_H

#include <qobject.h>


namespace Cervisia
{


class CommandBase : public QObject
{
    Q_OBJECT

public:
    enum ActionKind { Add, Remove, Update, SimulateUpdate, Commit, Edit, Unedit,
                      Lock, Unlock, Checkout, Other };

    CommandBase(const ActionKind& action);
    virtual ~CommandBase();

    ActionKind action() const { return m_action; }

    bool isRecursive() const { return m_recursive; }
    void setRecursive(bool recursive) { m_recursive = recursive; }

    virtual QString commandString() const = 0;

    virtual bool isRunning() const = 0;

    /**
     * cancel the current processing.
     */
    virtual void cancel() = 0;

    /**
     * checks if the line contains an error message.
     */
    virtual bool isErrorMessage(const QString& line) const = 0;

protected:
    void setAction(const ActionKind& action);

signals:
    void jobExited(bool normalExit, int status);    //TODO: rename?
    void receivedStdout(const QString& buffer);
    void receivedStderr(const QString& buffer);
    void receivedLine(const QString& line);

private:
    ActionKind m_action;
    bool       m_recursive;
};


}


#endif
