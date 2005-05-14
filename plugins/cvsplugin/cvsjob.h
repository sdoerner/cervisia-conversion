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

#ifndef CERVISIA_CVSJOB_H
#define CERVISIA_CVSJOB_H

#include <pluginjobbase.h>

class CvsJob_stub;
class DCOPRef;


namespace Cervisia
{

class CvsJob : public PluginJobBase
{
    Q_OBJECT

public:
    CvsJob(const DCOPRef& jobRef, const ActionKind& action);
    ~CvsJob();

    virtual QString commandString() const;

private:
    CvsJob_stub* m_cvsJob;
};


}


#endif
