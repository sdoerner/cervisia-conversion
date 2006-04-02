/*
 * Copyright (c) 2006 André Wöbbeking <Woebbeking@web.de>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef CERVISIA_EXTERNALDIFFCOMMAND_H
#define CERVISIA_EXTERNALDIFFCOMMAND_H


#include <svncommandbase.h>


namespace Cervisia
{


class ExternalDiffCommand : public SvnCommandBase
{
    Q_OBJECT

public:

    ExternalDiffCommand(const QString& diffApplication,
                        const QString& fileName,
                        const QString& revisionA,
                        const QString& revisionB);

    virtual ~ExternalDiffCommand();

    virtual bool prepare();

    virtual void execute();

private slots:

    void diff();

private:

    /**
     * The name of the diff application.
     */
    const QString m_diffApplication;

    /**
     * The name of the file to diff.
     */
    const QString m_fileName;

    /**
     * The first revision of the file to diff.
     */
    const QString m_revisionA;

    /**
     * The second revision of the file to diff.
     */
    const QString m_revisionB;

    /**
     * The name of the downloaded temporary file.
     */
    QString m_tempFileNameA;

    /**
     * The name of the downloaded temporary file.
     */
    QString m_tempFileNameB;
};


}


#endif
