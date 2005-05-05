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

#include "svnplugin.h"
using Cervisia::SvnPlugin;

#include <qdir.h>
#include <qfileinfo.h>

#include <dcopref.h>
#include <kapplication.h>
#include <kgenericfactory.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kurl.h>

#include <svnservice_stub.h>
#include <svnrepository_stub.h>

#include <kdebug.h>


typedef KGenericFactory<SvnPlugin> SvnPluginFactory;
K_EXPORT_COMPONENT_FACTORY( libsvnplugin, SvnPluginFactory( "svnplugin" ) )


SvnPlugin::SvnPlugin(QObject* parent, const char* name, const QStringList&)
    : PluginBase(parent, name)
    , m_svnService(0)
    , m_svnRepository(0)
{
    kdDebug() << "SvnPlugin::SvnPlugin()" << endl;

    startService();
}


SvnPlugin::~SvnPlugin()
{
    kdDebug() << "SvnPlugin::~SvnPlugin()" << endl;

    // stop the cvs DCOP service and delete reference
    if( m_svnService )
        m_svnService->quit();

    delete m_svnService;
}


QString SvnPlugin::type() const
{
    return "SVN";
}


DCOPRef SvnPlugin::service() const
{
    return DCOPRef(m_svnService->app(), m_svnService->obj());
}


bool SvnPlugin::canHandle(const KURL& workingCopy)
{
    const QFileInfo fi(workingCopy.path());

    QString path = fi.absFilePath() + "/.svn";

    // is this really a Subversion-controlled directory?
    QFileInfo svnDirInfo(path);

    return ( svnDirInfo.exists() &&
             svnDirInfo.isDir()  &&
             QFile::exists(path + "/entries") );
}


void SvnPlugin::setWorkingCopy(const KURL& workingCopy)
{
    bool opened = m_svnRepository->setWorkingCopy(workingCopy.path());
}


KURL SvnPlugin::workingCopy() const
{
    return KURL::fromPathOrURL(m_svnRepository->workingCopy());
}


QString SvnPlugin::repository() const
{
    return m_svnRepository->location();
}


void SvnPlugin::syncWithEntries(const QString& path)
{
    kdDebug() << "SvnPlugin::syncWithEntries(): path = " << path << endl;
    const QString fileName = path + QDir::separator() + ".svn/entries";

    QFile f(fileName);
    if( !f.open(IO_ReadOnly) )
        return;

    QDomDocument doc;
    if( !doc.setContent(&f) )
    {
        f.close();
        return;
    }
    f.close();

    QDomElement root = doc.documentElement();
    kdDebug(8050) << "SvnPlugin::syncWithEntries(): root = " << root.tagName() << endl;

    for( QDomNode node = root.firstChild(); !node.isNull(); node = node.nextSibling() )
    {
        if( node.isElement() )
        {
            QDomElement e = node.toElement();
            kdDebug(8050) << "SvnPlugin::syncWithEntries(): e = " << e.tagName() << endl;

            if( e.tagName() == "entry" )
            {
                Cervisia::Entry entry;

                const bool isDir = (e.attribute("kind") == "dir");

                entry.m_type = isDir ? Entry::Dir : Entry::File;
                entry.m_name = e.attribute("name");

                // ignore entries without name
                if( entry.m_name.isEmpty() )
                    continue;

                if( isDir )
                {
                    emit updateItem(entry);
                }
                else
                {
                    entry.m_revision = e.attribute("committed-rev");
                    emit updateItem(entry);
                }
            }
        }
    }
}


void SvnPlugin::startService()
{
    // start the cvs DCOP service
    QString error;
    QCString appId;

    if( KApplication::startServiceByDesktopName("svnservice", QStringList(), &error, &appId) )
    {
        KMessageBox::sorry(0, i18n("Starting svnservice failed with message: ") +
                error, "Cervisia");
    }
    else
    {
        // create a reference to the service
        m_svnService = new SvnService_stub(appId, "SvnService");
        m_svnRepository = new SvnRepository_stub(appId, "SvnRepository");
    }
}
