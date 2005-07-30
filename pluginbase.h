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

#ifndef CERVISIA_PLUGINBASE_H
#define CERVISIA_PLUGINBASE_H

#include <kparts/plugin.h>

#include "entry.h"

class DCOPRef;      // needed for service()


namespace Cervisia
{

class CommandBase;
class IgnoreFilterBase;
class PluginJobBase;
class SelectionIntf;


class PluginBase : public KParts::Plugin
{
    Q_OBJECT

public:
    PluginBase(QObject* parent, const char* name);
    ~PluginBase();

    /**
     * @return The KConfig object for the instance.
     */
    KConfig* config() const;

    void setFileView(SelectionIntf* fileView);

    virtual QString type() const = 0;
    virtual DCOPRef service() const = 0;    // FIXME: only temporary. remove me later!

    virtual bool canHandle(const KURL& workingCopy) = 0;
    virtual void setWorkingCopy(const KURL& workingCopy) = 0;
    virtual KURL workingCopy() const = 0;

    virtual QString repository() const = 0;

    virtual void syncWithEntries(const QString& path) = 0;

    virtual IgnoreFilterBase* filter(const QString& path) const = 0;

signals:
    void updateItem(const Cervisia::Entry& entry);
    void jobPrepared(Cervisia::PluginJobBase* job);
    void commandPrepared(Cervisia::CommandBase* cmd);

protected:
    SelectionIntf* m_fileView;
    PluginJobBase* m_currentJob;
};


}


#endif