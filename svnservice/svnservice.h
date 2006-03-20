/*
 * Copyright (c) 2005-2006 Christian Loose <christian.loose@kdemail.net>
 * Copyright (c) 2006 André Wöbbeking <Woebbeking@web.de>
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
 * Boston, MA  02110-1301  USA.
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
     * Shows information on who last modified each line of a file and when.
     *
     * @param fileName the name of the file to show annotations for
     * @param revision show annotations for this revision (number or tag)
     *
     * @return A DCOP reference to the svn job or in case of failure a
     *         null reference.
     */
    DCOPRef annotate(const QString& fileName, const QString& revision);

    /**
     */
    DCOPRef checkout(const QString& repository, const QString& revision,
                     const QString& workingFolder, bool recursive);

    /**
     */
    DCOPRef commit(const QStringList& files, const QString& commitMessage,
                   bool recursive);

    /**
     * Show differences between different revisions of a file.
     *
     * @param fileName the name of the file to show differences for
     * @param revisionA
     * @param revisionB
     * @param options
     *
     * @return A DCOP reference to the svn job or in case of failure a
     *         null reference.
     */
    DCOPRef diff(const QString& fileName,
                 const QString& revisionA,
                 const QString& revisionB,
                 const QStringList& options);

    /**
     * Download a revision of a file.
     *
     * @param fileName the name of the file to download
     * @param revision the revision to download
     * @param outputFile the local name of the downloaded file
     *
     * @return A DCOP reference to the svn job or in case of failure a
     *         null reference.
     */
    DCOPRef downloadRevision(const QString& fileName,
                             const QString& revision,
                             const QString& outputFile);

    /**
     * Download two revisions of a file in one job.
     *
     * @param fileName the name of the file to download
     * @param revisionA the first revision to download
     * @param outputFileA the local name of the downloaded file for the first revision
     * @param revisionB the second revision to download
     * @param outputFileB the local name of the downloaded file for the second revision
     *
     * @return A DCOP reference to the svn job or in case of failure a
     *         null reference.
     */
    DCOPRef downloadRevision(const QString& fileName,
                             const QString& revisionA,
                             const QString& outputFileA,
                             const QString& revisionB,
                             const QString& outputFileB);

    /**
     * Shows log messages for a file.
     *
     * @param fileName the name of the file to show log messages for
     *
     * @return A DCOP reference to the svn job or in case of failure a
     *         null reference.
     */
    DCOPRef log(const QString& fileName);

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
     * Merges changes from the repository into the files of the
     * working copy.
     *
     * @param files A list of files that should be updated.
     * @param recursive descend into subdirectories.
     *
     * @return A DCOP reference to the svn job or in case of failure a
     *         null reference.
     */
    DCOPRef update(const QStringList& files, bool recursive);

    /**
     * Quits the DCOP service.
     */
    void quit();

private:
    struct Private;
    Private* d;
};


#endif
