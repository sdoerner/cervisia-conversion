/*
 * Copyright (c) 2004-2005 Andr� W�bbeking <Woebbeking@web.de>
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


#ifndef CERVISIA_ENTRY_STATUS_CHANGE_H
#define CERVISIA_ENTRY_STATUS_CHANGE_H


#include <qstring.h>

#include "entry_status.h"


namespace Cervisia
{


/**
 * Dumb data struct to store a status change of an entry (i.e. for jobs like
 * status, update, ...).
 */
struct EntryStatusChange
{
    /**
     * The name of the changed entry (including the path inside the repository / working copy).
     */
    QString m_name;

    /**
     * The new status of the entry.
     */
    EntryStatus m_status;
};


} // namespace Cervisia


#endif // CERVISIA_ENTRY_STATUS_CHANGE_H
