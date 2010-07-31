/*
 * Copyright (c) 2002-2003 Christian Loose <christian.loose@hamburg.de>
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

#ifndef CVSJOB_H
#define CVSJOB_H

#include <tqobject.h>
#include <tqstring.h>
#include <tqstringlist.h>
#include <dcopobject.h>

class KProcess;


class KDE_EXPORT CvsJob : public TQObject, public DCOPObject
{
    Q_OBJECT
    K_DCOP

public:
    explicit CvsJob(unsigned jobNum);
    explicit CvsJob(const TQString& objId);
    virtual ~CvsJob();

    void clearCvsCommand();
    void setRSH(const TQString& rsh);
    void setServer(const TQString& server);
    void setDirectory(const TQString& directory);

    CvsJob& operator<<(const TQString& arg);
    CvsJob& operator<<(const char* arg);
    CvsJob& operator<<(const TQCString& arg);
    CvsJob& operator<<(const TQStringList& args);

k_dcop:
    bool execute();
    void cancel();

    bool isRunning() const;

    /**
     * Current cvs command.
     *
     * @return The current cvs command. Can be null if not set.
     */
    TQString cvsCommand() const;

    TQStringList output() const;

k_dcop_signals:
    void jobExited(bool normalExit, int status);
    void receivedStdout(const TQString& buffer);
    void receivedStderr(const TQString& buffer);

private slots:
    void slotProcessExited();
    void slotReceivedStdout(KProcess* proc, char* buffer, int buflen);
    void slotReceivedStderr(KProcess* proc, char* buffer, int buflen);

private:
    struct Private;
    Private* d;
};


#endif
