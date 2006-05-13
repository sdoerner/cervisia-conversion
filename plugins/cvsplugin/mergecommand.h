/*
 * Copyright (c) 2006 Christian Loose <christian.loose@kdemail.net>
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

#ifndef CERVISIA_MERGECOMMAND_H
#define CERVISIA_MERGECOMMAND_H

#include <cvscommandbase.h>

#include <qstringlist.h>


namespace Cervisia
{


class MergeCommand : public CvsCommandBase
{
public:
    MergeCommand(const QStringList& files);
    ~MergeCommand();

    void setCreateDirectories(bool createDirs);
    void setPruneDirectories(bool pruneDirs);

    virtual bool prepare();

private:
    QStringList m_fileList;
    bool        m_createDirectories;
    bool        m_pruneDirectories;
};


}


#endif
