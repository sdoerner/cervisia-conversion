/*
 * Copyright (c) 2005 André Wöbbeking <Woebbeking@web.de>
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

#ifndef CERVISIA_DIFFCOMMAND_H
#define CERVISIA_DIFFCOMMAND_H


#include "cvscommandbase.h"

#include <diff_info.h>

#include <qstringlist.h>


class DiffDialog;


namespace Cervisia
{


class DiffParser;


class DiffCommand : public CvsCommandBase
{
    Q_OBJECT

public:

    DiffCommand(const QString& fileName,
                const QString& revisionA,
                const QString& revisionB,
                const QStringList& options);

    virtual ~DiffCommand();

    const DiffInfoList& diffInfos() const;

    virtual bool prepare();
    virtual void execute();

private slots:

    void showDialog();

private:

    QString m_fileName;

    QString m_revisionA;

    QString m_revisionB;

    QStringList m_options;

    DiffParser* m_parser;

    DiffDialog* m_diffDialog;
};


}


#endif
