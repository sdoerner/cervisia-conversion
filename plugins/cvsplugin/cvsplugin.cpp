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

#include "cvsplugin.h"
using Cervisia::CvsPlugin;

#include <qdir.h>
#include <qfileinfo.h>

#include <dcopref.h>
#include <kapplication.h>
#include <kgenericfactory.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kurl.h>

#include <cvsjob_stub.h>
#include <cvsservice_stub.h>
#include <repository_stub.h>
#include <selectionintf.h>

#include "cvsjob.h"
#include "cvs_update_parser.h"
#include "addcommand.h"
#include "addwatchcommand.h"
#include "annotatecommand.h"
#include "commitcommand.h"
#include "editcommand.h"
#include "logcommand.h"
#include "removecommand.h"
#include "removewatchcommand.h"
#include "updatecommand.h"
#include "dirignorelist.h"
#include "globalignorelist.h"

#include <kdebug.h>


typedef KGenericFactory<CvsPlugin> CvsPluginFactory;
K_EXPORT_COMPONENT_FACTORY( libcvsplugin, CvsPluginFactory( "cvsplugin" ) )


CvsService_stub* CvsPlugin::m_cvsService = 0;
Cervisia::CvsUpdateParser* CvsPlugin::m_updateParser = 0;


CvsPlugin::CvsPlugin(QObject* parent, const char* name, const QStringList&)
    : PluginBase(parent, name)
    , m_cvsRepository(0)
{
    kdDebug(8050) << "CvsPlugin::CvsPlugin()" << endl;

    startService();
    setupMenuActions();
}


CvsPlugin::~CvsPlugin()
{
    kdDebug(8050) << "CvsPlugin::~CvsPlugin()" << endl;

    // stop the cvs DCOP service and delete reference
    if( m_cvsService )
        m_cvsService->quit();

    delete m_cvsService;
}


QString CvsPlugin::type() const
{
    return "CVS";
}


DCOPRef CvsPlugin::service() const
{
    return DCOPRef(m_cvsService->app(), m_cvsService->obj());
}


bool CvsPlugin::canHandle(const KURL& workingCopy)
{
    kdDebug(8050) << "CvsPlugin::canHandle(): url = " << workingCopy << endl;
    const QFileInfo fi(workingCopy.path());

    QString path = fi.absFilePath() + "/CVS";

    // is this really a cvs-controlled directory?
    QFileInfo cvsDirInfo(path);

    return ( cvsDirInfo.exists() &&
             cvsDirInfo.isDir() &&
             QFile::exists(path + "/Entries") &&
             QFile::exists(path + "/Repository") &&
             QFile::exists(path + "/Root") );
}


void CvsPlugin::setWorkingCopy(const KURL& workingCopy)
{
    bool opened = m_cvsRepository->setWorkingCopy(workingCopy.path());
}


KURL CvsPlugin::workingCopy() const
{
    return KURL::fromPathOrURL(m_cvsRepository->workingCopy());
}


QString CvsPlugin::repository() const
{
    return m_cvsRepository->location();
}


void CvsPlugin::syncWithEntries(const QString& filePath)
{
    kdDebug(8050) << "CvsPlugin::syncWithEntries(): path = " << filePath << endl;
    const QString path = filePath + QDir::separator();

    QFile f(path + "CVS/Entries");
    if( f.open(IO_ReadOnly) )
    {
        QTextStream stream(&f);
        while( !stream.eof() )
        {
            QString line = stream.readLine();

            Cervisia::Entry entry;

            const bool isDir(line[0] == 'D');

            if( isDir )
                line.remove(0, 1);

            if( line[0] != '/' )
                continue;

            entry.m_type = isDir ? Entry::Dir : Entry::File;
            entry.m_name = line.section('/', 1, 1);

            if (isDir)
            {
//                updateEntriesItem(entry, false);
                emit updateItem(entry);
            }
            else
            {
                QString rev(line.section('/', 2, 2));
                const QString timestamp(line.section('/', 3, 3));
                const QString options(line.section('/', 4, 4));
                entry.m_tag = line.section('/', 5, 5);

//                const bool isBinary(options.find("-kb") >= 0);

                // file date in local time
                entry.m_dateTime = QFileInfo(path + entry.m_name).lastModified();

                if( rev == "0" )
                    entry.m_status = Cervisia::LocallyAdded;
                else if( rev.length() > 2 && rev[0] == '-' )
                {
                    entry.m_status = Cervisia::LocallyRemoved;
                    rev.remove(0, 1);
                }
                else if (timestamp.find('+') >= 0)
                {
                    entry.m_status = Cervisia::Conflict;
                }
                else
                {
                    const QDateTime date(QDateTime::fromString(timestamp)); // UTC Time
                    QDateTime fileDateUTC;
                    fileDateUTC.setTime_t(entry.m_dateTime.toTime_t(), Qt::UTC);
                    if (date != fileDateUTC)
                        entry.m_status = Cervisia::LocallyModified;
                }

                entry.m_revision = rev;

//                updateEntriesItem(entry, isBinary);
                emit updateItem(entry);
            }
        }
    }
}


Cervisia::IgnoreFilterBase* CvsPlugin::filter(const QString& path) const
{
    return new DirIgnoreList(path, new GlobalIgnoreList);
}


Cervisia::UpdateParser* CvsPlugin::updateParser() const
{
    return m_updateParser;
}


void CvsPlugin::annotate(const QString& fileName, const QString& revision)
{
    executeCommand(new AnnotateCommand(fileName, revision));	
}


void CvsPlugin::add()
{
    kdDebug(8050) << "CvsPlugin::add()" << endl;

    QStringList selectionList = m_fileView->multipleSelection();
    if( selectionList.isEmpty() )
        return;

    executeCommand(new AddCommand(selectionList));
}


void CvsPlugin::addBinary()
{
    kdDebug(8050) << "CvsPlugin::addBinary()" << endl;

    QStringList selectionList = m_fileView->multipleSelection();
    if( selectionList.isEmpty() )
        return;

    AddCommand* cmd = new AddCommand(selectionList);
    cmd->setBinary(true);

    executeCommand(cmd);
}


void CvsPlugin::addWatch()
{
    kdDebug(8050) << "CvsPlugin::addWatch()" << endl;

    QStringList selectionList = m_fileView->multipleSelection();
    if( selectionList.isEmpty() )
        return;

    executeCommand(new AddWatchCommand(selectionList));
}


void CvsPlugin::annotate()
{
    kdDebug(8050) << "CvsPlugin::annotate()" << endl;

    QString fileName = m_fileView->singleSelection();
    if( fileName.isEmpty() )
        return;

    executeCommand(new AnnotateCommand(fileName));
}


void CvsPlugin::commit()
{
    kdDebug(8050) << "CvsPlugin::commit()" << endl;

    QStringList selectionList = m_fileView->multipleSelection();
    if( selectionList.isEmpty() )
        return;

    CommitCommand* cmd = new CommitCommand(selectionList);
//     cmd->setRecursive(opt_commitRecursive);

    executeCommand(cmd);
}


void CvsPlugin::edit()
{
    kdDebug(8050) << "CvsPlugin::edit()" << endl;

    QStringList selectionList = m_fileView->multipleSelection();
    if( selectionList.isEmpty() )
        return;

    executeCommand(new EditCommand(selectionList, EditCommand::Edit));
}


void CvsPlugin::log()
{
    kdDebug(8050) << "CvsPlugin::log()" << endl;

    QString fileName = m_fileView->singleSelection();
    if( fileName.isEmpty() )
        return;

    executeCommand(new LogCommand(fileName));
}


void CvsPlugin::remove()
{
    kdDebug(8050) << "CvsPlugin::remove()" << endl;

    QStringList selectionList = m_fileView->multipleSelection();
    if( selectionList.isEmpty() )
        return;

    RemoveCommand* cmd = new RemoveCommand(selectionList);
//     cmd->setRecursive(opt_commitRecursive);

    executeCommand(cmd);
}


void CvsPlugin::removeWatch()
{
    kdDebug(8050) << "CvsPlugin::removeWatch()" << endl;

    QStringList selectionList = m_fileView->multipleSelection();
    if( selectionList.isEmpty() )
        return;

    executeCommand(new RemoveWatchCommand(selectionList));
}


void CvsPlugin::revert()
{
    kdDebug(8050) << "CvsPlugin::revert()" << endl;

    QStringList selectionList = m_fileView->multipleSelection();
    if( selectionList.isEmpty() )
        return;

    UpdateCommand* cmd = new UpdateCommand(selectionList, m_updateParser);
//     cmd->setRecursive(opt_updateRecursive);
    cmd->setExtraOption("-C");

    executeCommand(cmd);
}


void CvsPlugin::simulateUpdate()
{
    kdDebug(8050) << "CvsPlugin::simulateUpdate()" << endl;

    QStringList selectionList = m_fileView->multipleSelection();
    if( selectionList.isEmpty() )
        return;

    UpdateCommand* cmd = new UpdateCommand(selectionList, m_updateParser);
//     cmd->setRecursive(opt_updateRecursive);
    cmd->setSimulation(true);

    executeCommand(cmd);
}


void CvsPlugin::unedit()
{
    kdDebug(8050) << "CvsPlugin::unedit()" << endl;

    QStringList selectionList = m_fileView->multipleSelection();
    if( selectionList.isEmpty() )
        return;

    executeCommand(new EditCommand(selectionList, EditCommand::Unedit));
}


void CvsPlugin::update()
{
    kdDebug(8050) << "CvsPlugin::update()" << endl;

    QStringList selectionList = m_fileView->multipleSelection();
    if( selectionList.isEmpty() )
        return;

    UpdateCommand* cmd = new UpdateCommand(selectionList, m_updateParser);
//     cmd->setRecursive(opt_updateRecursive);

    executeCommand(cmd);
}


void CvsPlugin::updateToHead()
{
    kdDebug(8050) << "CvsPlugin::updateToHead()" << endl;

    QStringList selectionList = m_fileView->multipleSelection();
    if( selectionList.isEmpty() )
        return;

    UpdateCommand* cmd = new UpdateCommand(selectionList, m_updateParser);
//     cmd->setRecursive(opt_updateRecursive);
    cmd->setExtraOption("-A");

    executeCommand(cmd);
}


void CvsPlugin::executeCommand(CvsCommandBase* cmd)
{
    if( cmd->prepare() )
    {
        emit commandPrepared(cmd);
        cmd->execute();
    }
}


void CvsPlugin::setupMenuActions()
{
    KAction* action;
    QString  hint;

    actionCollection()->setHighlightingEnabled(true);

    //
    // File Menu
    //
    action = new KAction( i18n("&Update"), "vcs_update", CTRL+Key_U,
                          this, SLOT( update() ),
                          actionCollection(), "file_update" );
    hint = i18n("Updates the selected files and folders (cvs update)");
    action->setToolTip(hint);
    action->setWhatsThis(hint);

    action = new KAction( i18n("&Status"), "vcs_status", Key_F5,
                          this, SLOT( simulateUpdate() ),
                          actionCollection(), "file_status" );
    hint = i18n("Updates the status of the selected files and folders (cvs -n update)");
    action->setToolTip(hint);
    action->setWhatsThis(hint);

    action = new KAction( i18n("&Commit..."), "vcs_commit", Key_NumberSign,
                          this, SLOT( commit() ),
                          actionCollection(), "file_commit" );
    hint = i18n("Commits the selected files (cvs commit)");
    action->setToolTip(hint);
    action->setWhatsThis(hint);

    action = new KAction( i18n("&Add to Repository..."), "vcs_add", Key_Insert,
                          this, SLOT( add() ),
                          actionCollection(), "file_add" );
    hint = i18n("Adds the selected files to the repository (cvs add)");
    action->setToolTip(hint);
    action->setWhatsThis(hint);

    action = new KAction( i18n("Add &Binary..."), 0,
                          this, SLOT( addBinary() ),
                          actionCollection(), "file_add_binary" );
    hint = i18n("Adds the selected files as binaries to the repository (cvs -kb add)");
    action->setToolTip(hint);
    action->setWhatsThis(hint);

    action = new KAction( i18n("&Remove From Repository..."), "vcs_remove", Key_Delete,
                          this, SLOT( remove() ),
                          actionCollection(), "file_remove" );
    hint = i18n("Removes the selected files from the repository (cvs remove)");
    action->setToolTip(hint);
    action->setWhatsThis(hint);

    action = new KAction( i18n("Rever&t"), 0,
                          this, SLOT( revert() ),
                          actionCollection(), "file_revert_local_changes" );
    hint = i18n("Reverts the selected files (cvs update -C / only cvs 1.11)");
    action->setToolTip(hint);
    action->setWhatsThis(hint);

    //
    // View Menu
    //
    action = new KAction( i18n("Browse &Log..."), CTRL+Key_L,
                          this, SLOT( log() ),
                          actionCollection(), "view_log" );
    hint = i18n("Shows the revision tree of the selected file (cvs log)");
    action->setToolTip(hint);
    action->setWhatsThis(hint);

    action = new KAction( i18n("&Annotate..."), CTRL+Key_A,
                          this, SLOT( annotate() ),
                          actionCollection(), "view_annotate" );
    hint = i18n("Shows a blame-annotated view of the selected file");
    action->setToolTip(hint);
    action->setWhatsThis(hint);

    //
    // Advanced Menu
    //
    action = new KAction( i18n("Update to &HEAD"), 0,
                          this, SLOT( updateToHead() ),
                          actionCollection(), "update_to_head" );
    hint = i18n("Updates the selected files to the HEAD revision (cvs update -A)");
    action->setToolTip(hint);
    action->setWhatsThis(hint);

    action = new KAction( i18n("&Add Watch..."), 0,
                          this, SLOT( addWatch() ),
                          actionCollection(), "add_watch" );
    hint = i18n("Adds a watch for the selected files");
    action->setToolTip(hint);
    action->setWhatsThis(hint);

    action = new KAction( i18n("&Remove Watch..."), 0,
                          this, SLOT( removeWatch() ),
                          actionCollection(), "remove_watch" );
    hint = i18n("Removes a watch from the selected files");
    action->setToolTip(hint);
    action->setWhatsThis(hint);

    action = new KAction( i18n("Ed&it Files"), 0,
                          this, SLOT( edit() ),
                          actionCollection(), "edit_files" );
    hint = i18n("Edits the selected files (cvs edit)");
    action->setToolTip(hint);
    action->setWhatsThis(hint);

    action = new KAction( i18n("U&nedit Files"), 0,
                          this, SLOT( unedit() ),
                          actionCollection(), "unedit_files" );
    hint = i18n("Unedits the selected files (cvs unedit)");
    action->setToolTip(hint);
    action->setWhatsThis(hint);
}


void CvsPlugin::startService()
{
    kdDebug(8050) << "CvsPlugin::startService(): start cvs DCOP service" << endl;

    // start the cvs DCOP service
    QString error;
    QCString appId;

    if( KApplication::startServiceByDesktopName("cvsservice", QStringList(), &error, &appId) )
    {
        KMessageBox::sorry(0, i18n("Starting cvsservice failed with message: ") +
                error, "Cervisia");
    }
    else
    {
        // create a reference to the service
        m_cvsService = new CvsService_stub(appId, "CvsService");
        m_cvsRepository = new Repository_stub(appId, "CvsRepository");
        m_updateParser = new CvsUpdateParser();
    }
}

#include "cvsplugin.moc"
