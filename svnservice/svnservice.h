/*
 * Copyright (c) 2005 Christian Loose <christian.loose@kdemail.net>
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
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */

#ifndef SVNSERVICE_H
#define SVNSERVICE_H

#include <qstringlist.h>
#include <dcopref.h>
#include <dcopobject.h>

class QString;


class KDE_EXPORT SvnService : public DCOPObject
{
    K_DCOP

public:
    SvnService();
    ~SvnService();

k_dcop:
    /**
     */
    DCOPRef add(const QStringList& files);

    /**
     */
    DCOPRef commit(const QStringList& files, const QString& commitMessage,
                   bool recursive);

    /**
     */
    DCOPRef remove(const QStringList& files);

    /**
     */
    DCOPRef revert(const QStringList& files, bool recursive);

    /**
     */
    DCOPRef simulateUpdate(const QStringList& files, bool recursive);

    /**
     * Quits the DCOP service.
     */
    void quit();

private:
    struct Private;
    Private* d;
};


#endif
