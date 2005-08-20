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

#include <selectionintf.h>
#include <svnservice_stub.h>
#include <svnrepository_stub.h>

#include "svnpluginsettings.h"
#include "globalignorelist.h"
#include "addcommand.h"
#include "annotatecommand.h"
#include "commitcommand.h"
#include "logcommand.h"
#include "removecommand.h"
#include "updatecommand.h"
#include "svn_update_parser.h"

#include <kdebug.h>


typedef KGenericFactory<SvnPlugin> SvnPluginFactory;
K_EXPORT_COMPONENT_FACTORY( libsvnplugin, SvnPluginFactory( "svnplugin" ) )


SvnService_stub* SvnPlugin::m_svnService = 0;
Cervisia::SvnUpdateParser* SvnPlugin::m_updateParser = 0;


SvnPlugin::SvnPlugin(QObject* parent, const char* name, const QStringList&)
    : PluginBase(parent, name)
    , m_svnRepository(0)
{
    kdDebug(8050) << "SvnPlugin::SvnPlugin()" << endl;

    startService();
    setupMenuActions();
}


SvnPlugin::~SvnPlugin()
{
    kdDebug(8050) << "SvnPlugin::~SvnPlugin()" << endl;

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
    kdDebug(8050) << "SvnPlugin::canHandle(): url = " << workingCopy << endl;

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
    /*bool opened = */m_svnRepository->setWorkingCopy(workingCopy.path());
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
    kdDebug(8050) << "SvnPlugin::syncWithEntries(): path = " << path << endl;
    const QString fileName = path + QDir::separator() + ".svn/entries";

    QFile f(fileName);
    if( !f.open(IO_ReadOnly) )
        return;

    QDomDocument doc;

    // FIXME: retrieve error message from setContent() method
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
//            kdDebug(8050) << "SvnPlugin::syncWithEntries(): e = " << e.tagName() << endl;

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

                    // retrieve last modified date from file (local time)
                    entry.m_dateTime = QFileInfo(path + QDir::separator() + entry.m_name).lastModified();

                    const QString schedule       = e.attribute("schedule");
                    const QString conflictWrk    = e.attribute("conflict-wrk");
                    const QString textTimeAttr   = e.attribute("text-time");
                    const QDateTime textDateTime = QDateTime::fromString(textTimeAttr, Qt::ISODate);

                    QDateTime fileDateUTC;
                    fileDateUTC.setTime_t(entry.m_dateTime.toTime_t(), Qt::UTC);

                    if( schedule == "add" )
                        entry.m_status = Cervisia::LocallyAdded;
                    else if( schedule == "delete" )
                        entry.m_status = Cervisia::LocallyRemoved;
                    else if( !conflictWrk.isEmpty() )
                        entry.m_status = Cervisia::Conflict;
                    else if( textDateTime != fileDateUTC )
                        entry.m_status = Cervisia::LocallyModified;

                    emit updateItem(entry);
                }
            }
        }
    }
}


Cervisia::IgnoreFilterBase* SvnPlugin::filter(const QString& path) const
{
    return new GlobalIgnoreList;
}


Cervisia::UpdateParser* SvnPlugin::updateParser() const
{
    return m_updateParser;
}


void SvnPlugin::annotate(const QString& fileName, const QString& revision)
{
}


void SvnPlugin::add()
{
    kdDebug(8050) << "SvnPlugin::add()" << endl;

    QStringList selectionList = m_fileView->multipleSelection();
    if( selectionList.isEmpty() )
        return;

    executeCommand(new AddCommand(selectionList));
}


void SvnPlugin::annotate()
{
    kdDebug(8050) << "CvsPlugin::annotate()" << endl;

    QString fileName = m_fileView->singleSelection();
    if( fileName.isEmpty() )
        return;

    executeCommand(new AnnotateCommand(fileName));
}


void SvnPlugin::commit()
{
    kdDebug(8050) << "SvnPlugin::commit()" << endl;

    QStringList selectionList = m_fileView->multipleSelection();
    if( selectionList.isEmpty() )
        return;

    CommitCommand* cmd = new CommitCommand(selectionList);
    cmd->setRecursive(SvnPluginSettings::commitRecursive());

    executeCommand(cmd);
}


void SvnPlugin::log()
{
    kdDebug(8050) << "SvnPlugin::log()" << endl;

    QString fileName = m_fileView->singleSelection();
    if( fileName.isEmpty() )
        return;

    executeCommand(new LogCommand(fileName, this));
}


void SvnPlugin::remove()
{
    kdDebug(8050) << "SvnPlugin::remove()" << endl;

    QStringList selectionList = m_fileView->multipleSelection();
    if( selectionList.isEmpty() )
        return;

    executeCommand(new RemoveCommand(selectionList));
}


void SvnPlugin::simulateUpdate()
{
    kdDebug(8050) << "SvnPlugin::simulateUpdate()" << endl;

    QStringList selectionList = m_fileView->multipleSelection();
    if( selectionList.isEmpty() )
        return;

    UpdateCommand* cmd = new UpdateCommand(selectionList, m_updateParser);
    cmd->setRecursive(SvnPluginSettings::updateRecursive());
    cmd->setSimulation(true);

    executeCommand(cmd);
}


void SvnPlugin::commitRecursive()
{
    bool recursive = SvnPluginSettings::commitRecursive();
    SvnPluginSettings::setCommitRecursive(!recursive);

    SvnPluginSettings::writeConfig();
}


void SvnPlugin::updateRecursive()
{
    bool recursive = SvnPluginSettings::updateRecursive();
    SvnPluginSettings::setUpdateRecursive(!recursive);

    SvnPluginSettings::writeConfig();
}


void SvnPlugin::executeCommand(SvnCommandBase* cmd)
{
    if( cmd->prepare() )
    {
        emit commandPrepared(cmd);
        cmd->execute();
    }
}


void SvnPlugin::setupMenuActions()
{
    KAction* action;
    QString  hint;

    actionCollection()->setHighlightingEnabled(true);

    //
    // File Menu
    //
    action = new KAction( i18n("&Status"), "vcs_status", Key_F5,
                          this, SLOT( simulateUpdate() ),
                          actionCollection(), "file_status" );
    hint = i18n("Updates the status of the selected files and folders (svn status)");
    action->setToolTip(hint);
    action->setWhatsThis(hint);

    action = new KAction( i18n("&Commit..."), "vcs_commit", Key_NumberSign,
                          this, SLOT( commit() ),
                          actionCollection(), "file_commit" );
    hint = i18n("Commits the selected files (svn commit)");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    action = new KAction( i18n("&Add to Repository..."), "vcs_add", Key_Insert,
                          this, SLOT( add() ),
                          actionCollection(), "file_add" );
    hint = i18n("Adds the selected files to the repository (svn add)");
    action->setToolTip(hint);
    action->setWhatsThis(hint);

    action = new KAction( i18n("&Remove From Repository..."), "vcs_remove", Key_Delete,
                          this, SLOT( remove() ),
                          actionCollection(), "file_remove" );
    hint = i18n("Removes the selected files from the repository (svn delete)");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    //
    // View Menu
    //
    action = new KAction( i18n("Browse &Log..."), CTRL+Key_L,
                          this, SLOT( log() ),
                          actionCollection(), "view_log" );
    hint = i18n("Shows the revision tree of the selected file (svn log)");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    action = new KAction( i18n("&Annotate..."), CTRL+Key_A,
                          this, SLOT( annotate() ),
                          actionCollection(), "view_annotate" );
    hint = i18n("Shows a blame-annotated view of the selected file");
    action->setToolTip(hint);
    action->setWhatsThis(hint);

    //
    // Settings Menu
    //
    KToggleAction* toggleAction = new KToggleAction(
                                i18n("&Update Recursively"), 0,
                                this, SLOT( updateRecursive() ),
                                actionCollection(), "settings_update_recursively" );
    hint = i18n("Determines whether updates are recursive");
    toggleAction->setToolTip(hint);
    toggleAction->setWhatsThis(hint);
    toggleAction->setChecked(SvnPluginSettings::updateRecursive());

    toggleAction = new KToggleAction( i18n("C&ommit && Remove Recursively"), 0,
                                      this, SLOT( commitRecursive() ),
                                      actionCollection(), "settings_commit_recursively" );
    hint = i18n("Determines whether commits and removes are recursive");
    toggleAction->setToolTip(hint);
    toggleAction->setWhatsThis(hint);
    toggleAction->setChecked(SvnPluginSettings::commitRecursive());
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
        m_updateParser = new SvnUpdateParser();
    }
}

#include "svnplugin.moc"
