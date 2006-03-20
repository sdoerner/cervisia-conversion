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

#ifndef CERVISIA_CHECKOUTCOMMAND_H
#define CERVISIA_CHECKOUTCOMMAND_H

#include <svncommandbase.h>


namespace Cervisia
{


class CheckoutCommand : public SvnCommandBase
{
public:
    CheckoutCommand(const QString& workingFolder, const QString& repository);
    ~CheckoutCommand();

    void setRevision(const QString& revision);

    virtual bool prepare();

private:
    QString m_workingFolder;
    QString m_repository;
    QString m_revision;
};


}


#endif
