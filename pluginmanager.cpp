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
#include <kparts/mainwindow.h>
#include <kstatusbar.h>
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
//     kdDebug(8050) << k_funcinfo << endl;
}


PluginManager::~PluginManager()
{
//     kdDebug(8050) << k_funcinfo << endl;
}


void PluginManager::setPart(KParts::Part* part)
{
//     kdDebug(8050) << k_funcinfo << endl;

    m_part = part;

    // get the list of all KPart plugins
    QPtrList<KParts::Plugin> pluginList = KParts::Plugin::pluginObjects(m_part);

    kdDebug(8050) << k_funcinfo << " plugins count = " << pluginList.count() << endl;

    // remove all plugins from main menu
    if( m_part->factory() )
    {
        PluginBase* plugin = static_cast<PluginBase*>(pluginList.first());
        for( ; plugin; plugin = static_cast<PluginBase*>(pluginList.next()) )
        {
            m_part->factory()->removeClient(plugin);
            m_plugins.push_back(plugin);
        }
    }
}


Cervisia::PluginList PluginManager::plugins() const
{
    return m_plugins;
}


Cervisia::PluginBase* PluginManager::pluginForUrl(const KURL& url)
{
    PluginBase* result = 0;

    PluginList::ConstIterator it;
    for( it = m_plugins.begin(); it != m_plugins.end(); ++it )
    {
        PluginBase* plugin = *it;

        kdDebug(8050) << k_funcinfo << " type = " << plugin->type()
                                    << ", url = " << url.prettyURL() << endl;

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

            // show menu item hints in statusbar
            KParts::MainWindow* win = static_cast<KParts::MainWindow*>(m_part->parent());
            if( win )
            {
                KStatusBar* statusBar = win->statusBar();

                statusBar->connect(
                    plugin->actionCollection(), SIGNAL(actionStatusText(const QString &)),
                    SLOT(message(const QString &)) );
                statusBar->connect(
                    plugin->actionCollection(), SIGNAL(clearStatusText()),
                    SLOT(clear()) );
            }

            plugin->setWorkingCopy(url);
            m_currentPlugin = result = plugin;
            break;
        }
    }

    return result;
}


Cervisia::PluginBase* PluginManager::pluginForType(const QString& type)
{
    PluginBase* result = 0;

    PluginList::ConstIterator it;
    for( it = m_plugins.begin(); it != m_plugins.end(); ++it )
    {
        if( (*it)->type() == type )
        {
            result = *it;
            break;
        }
    }

    return result;
}


Cervisia::PluginBase* PluginManager::currentPlugin() const
{
    return m_currentPlugin;
}
