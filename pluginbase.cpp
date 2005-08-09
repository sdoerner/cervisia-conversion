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
 * Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "pluginbase.h"
using Cervisia::PluginBase;

#include "selectionintf.h"


PluginBase::PluginBase(QObject* parent, const char* name)
    : Plugin(parent, name)
{
}


PluginBase::~PluginBase()
{
}


KConfig* PluginBase::config() const
{
    if (const KInstance* kinstance = instance())
        return kinstance->config();

    return 0;
}


void PluginBase::setFileView(Cervisia::SelectionIntf* fileView)
{
    m_fileView = fileView;
}

#include "pluginbase.moc"
