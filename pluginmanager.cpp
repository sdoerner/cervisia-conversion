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

#include "pluginmanager.h"
using Cervisia::PluginManager;

#include <kdebug.h>
#include <kstaticdeleter.h>
#include <kxmlguifactory.h>

#include "pluginbase.h"


PluginManager* PluginManager::m_self = 0;
static KStaticDeleter<PluginManager> staticDeleter;

PluginManager* PluginManager::self()
{
    if( !m_self )
        staticDeleter.setObject(m_self, new PluginManager());

    return m_self;
}


PluginManager::PluginManager()
    : m_part(0)
    , m_currentPlugin(0)
{
    kdDebug() << "PluginManager::PluginManager()" << endl;
}


PluginManager::~PluginManager()
{
    kdDebug() << "PluginManager::~PluginManager()" << endl;
}


void PluginManager::setPart(KParts::Part* part)
{
    kdDebug() << "PluginManager::setPart()" << endl;

    m_part = part;

    // get the list of all KPart plugins
    m_pluginList = KParts::Plugin::pluginObjects(m_part);

    kdDebug() << "PluginManager::setPart(): plugins count = " << m_pluginList.count() << endl;

    // remove all plugins from main menu
    if( m_part->factory() )
    {
        KParts::Plugin* plugin = m_pluginList.first();
        for( ; plugin; plugin = m_pluginList.next() )
        {
            m_part->factory()->removeClient(plugin);
        }
    }
}


Cervisia::PluginBase* PluginManager::pluginForUrl(const KURL& url)
{
    PluginBase* result = 0;

    Cervisia::PluginBase* plugin = static_cast<Cervisia::PluginBase*>(m_pluginList.first());
    for( ; plugin; plugin = static_cast<Cervisia::PluginBase*>(m_pluginList.next()) )
    {
        kdDebug() << "PluginManager::pluginForUrl(): type = " << plugin->type() << ", url = " << url.prettyURL() << endl;
        if( plugin->canHandle(url) )
        {
            // is the plugin already active? --> no need to change the menu
            if( m_currentPlugin && m_currentPlugin->type() == plugin->type() )
            {
                plugin->setWorkingCopy(url);
                return m_currentPlugin;
            }

            if( m_currentPlugin )
                m_part->factory()->removeClient(m_currentPlugin);

            m_part->factory()->addClient(plugin);
            plugin->setWorkingCopy(url);
            m_currentPlugin = result = plugin;
            break;
        }
    }

    return result;
}


Cervisia::PluginBase* PluginManager::currentPlugin() const
{
    return m_currentPlugin;
}
