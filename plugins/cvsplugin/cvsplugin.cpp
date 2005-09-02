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

#include "cvspluginsettings.h"
#include "cvs_update_parser.h"
#include "addcommand.h"
#include "addwatchcommand.h"
#include "annotatecommand.h"
#include "commitcommand.h"
#include "createtagcommand.h"
#include "deletetagcommand.h"
#include "diffcommand.h"
#include "editcommand.h"
#include "lockcommand.h"
#include "logcommand.h"
#include "removecommand.h"
#include "removewatchcommand.h"
#include "updatecommand.h"
#include "updatetagcommand.h"
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
    kdDebug(8050) << k_funcinfo << endl;

    startService();
    setupMenuActions();
}


CvsPlugin::~CvsPlugin()
{
    kdDebug(8050) << k_funcinfo << endl;

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
    kdDebug(8050) << k_funcinfo << "url = " << workingCopy << endl;
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
    /*bool opened =*/ m_cvsRepository->setWorkingCopy(workingCopy.path());
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
    kdDebug(8050) << k_funcinfo << "path = " << filePath << endl;
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
                emit updateItem(entry);
            }
            else
            {
                QString rev(line.section('/', 2, 2));
                const QString timestamp(line.section('/', 3, 3));

                // file contains binary data?
                const QString options(line.section('/', 4, 4));
                entry.m_binary = (options.find("-kb") >= 0);

                entry.m_tag = line.section('/', 5, 5);

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
    kdDebug(8050) << k_funcinfo << endl;

    if( fileName.isEmpty() )
        return;

    executeCommand(new AnnotateCommand(fileName, revision));	
}


void CvsPlugin::add()
{
    kdDebug(8050) << k_funcinfo << endl;

    QStringList selectionList = m_fileView->multipleSelection();
    if( selectionList.isEmpty() )
        return;

    executeCommand(new AddCommand(selectionList));
}


void CvsPlugin::addBinary()
{
    kdDebug(8050) << k_funcinfo << endl;

    QStringList selectionList = m_fileView->multipleSelection();
    if( selectionList.isEmpty() )
        return;

    AddCommand* cmd = new AddCommand(selectionList);
    cmd->setBinary(true);

    executeCommand(cmd);
}


void CvsPlugin::addWatch()
{
    kdDebug(8050) << k_funcinfo << endl;

    QStringList selectionList = m_fileView->multipleSelection();
    if( selectionList.isEmpty() )
        return;

    executeCommand(new AddWatchCommand(selectionList));
}


void CvsPlugin::annotate()
{
    kdDebug(8050) << k_funcinfo << endl;

    annotate(m_fileView->singleSelection());
}


void CvsPlugin::commit()
{
    kdDebug(8050) << k_funcinfo << endl;

    QStringList selectionList = m_fileView->multipleSelection();
    if( selectionList.isEmpty() )
        return;

    CommitCommand* cmd = new CommitCommand(selectionList);
    cmd->setRecursive(CvsPluginSettings::commitRecursive());

    executeCommand(cmd);
}


void CvsPlugin::createTag()
{
    kdDebug(8050) << k_funcinfo << endl;

    QStringList selectionList = m_fileView->multipleSelection();
    if( selectionList.isEmpty() )
        return;

    executeCommand(new CreateTagCommand(selectionList));
}


void CvsPlugin::deleteTag()
{
    kdDebug(8050) << k_funcinfo << endl;

    QStringList selectionList = m_fileView->multipleSelection();
    if( selectionList.isEmpty() )
        return;

    executeCommand(new DeleteTagCommand(selectionList));
}


void CvsPlugin::diff(const QString& fileName,
                     const QString& revisionA,
                     const QString& revisionB)
{
    kdDebug(8050) << k_funcinfo << endl;

    if( fileName.isEmpty() )
        return;

    executeCommand(new DiffCommand(fileName, revisionA, revisionB, QStringList()));	
}


void CvsPlugin::diffToBase()
{
    kdDebug(8050) << k_funcinfo << endl;

    diff(m_fileView->singleSelection(), "BASE", QString::null);
}


void CvsPlugin::diffToHead()
{
    kdDebug(8050) << k_funcinfo << endl;

    diff(m_fileView->singleSelection(), "HEAD", QString::null);
}


void CvsPlugin::edit()
{
    kdDebug(8050) << k_funcinfo << endl;

    QStringList selectionList = m_fileView->multipleSelection();
    if( selectionList.isEmpty() )
        return;

    executeCommand(new EditCommand(selectionList, EditCommand::Edit));
}


void CvsPlugin::lock()
{
    kdDebug(8050) << k_funcinfo << endl;

    QStringList selectionList = m_fileView->multipleSelection();
    if( selectionList.isEmpty() )
        return;

    executeCommand(new LockCommand(selectionList, LockCommand::Lock));
}


void CvsPlugin::log()
{
    kdDebug(8050) << k_funcinfo << endl;

    QString fileName = m_fileView->singleSelection();
    if( fileName.isEmpty() )
        return;

    executeCommand(new LogCommand(fileName, this));
}


void CvsPlugin::remove()
{
    kdDebug(8050) << k_funcinfo << endl;

    QStringList selectionList = m_fileView->multipleSelection();
    if( selectionList.isEmpty() )
        return;

    RemoveCommand* cmd = new RemoveCommand(selectionList);
    cmd->setRecursive(CvsPluginSettings::commitRecursive());

    executeCommand(cmd);
}


void CvsPlugin::removeWatch()
{
    kdDebug(8050) << k_funcinfo << endl;

    QStringList selectionList = m_fileView->multipleSelection();
    if( selectionList.isEmpty() )
        return;

    executeCommand(new RemoveWatchCommand(selectionList));
}


void CvsPlugin::revert()
{
    kdDebug(8050) << k_funcinfo << endl;

    QStringList selectionList = m_fileView->multipleSelection();
    if( selectionList.isEmpty() )
        return;

    UpdateCommand* cmd = new UpdateCommand(selectionList, m_updateParser);
    cmd->setRecursive(CvsPluginSettings::updateRecursive());
    cmd->setCreateDirectories(CvsPluginSettings::createDirectories());
    cmd->setPruneDirectories(CvsPluginSettings::pruneDirectories());
    cmd->setExtraOption("-C");

    executeCommand(cmd);
}


void CvsPlugin::showLastChange()
{
    QString fileName, revisionA;
    m_fileView->getSingleSelection(&fileName, &revisionA);
    if (fileName.isEmpty())
        return;

    const int pos = revisionA.findRev('.') + 1;
    bool ok;
    const unsigned int lastNumber(revisionA.mid(pos).toUInt(&ok));
    if ((pos <= 0) || !ok)
    {
        KMessageBox::sorry(0,
                           i18n("The revision looks invalid."),
                           "Cervisia");
        return;
    }
    if (!lastNumber)
    {
        KMessageBox::sorry(0,
                           i18n("This is the first revision of the branch."),
                           "Cervisia");
        return;
    }
    const QString revisionB = revisionA.left(pos) + QString::number(lastNumber - 1);

    executeCommand(new DiffCommand(fileName, revisionB, revisionA, QStringList()));
}


void CvsPlugin::simulateUpdate()
{
    kdDebug(8050) << k_funcinfo << endl;

    QStringList selectionList = m_fileView->multipleSelection();
    if( selectionList.isEmpty() )
        return;

    UpdateCommand* cmd = new UpdateCommand(selectionList, m_updateParser);
    cmd->setRecursive(CvsPluginSettings::updateRecursive());
    cmd->setCreateDirectories(CvsPluginSettings::createDirectories());
    cmd->setPruneDirectories(CvsPluginSettings::pruneDirectories());
    cmd->setSimulation(true);

    executeCommand(cmd);
}


void CvsPlugin::unedit()
{
    kdDebug(8050) << k_funcinfo << endl;

    QStringList selectionList = m_fileView->multipleSelection();
    if( selectionList.isEmpty() )
        return;

    executeCommand(new EditCommand(selectionList, EditCommand::Unedit));
}


void CvsPlugin::unlock()
{
    kdDebug(8050) << k_funcinfo << endl;

    QStringList selectionList = m_fileView->multipleSelection();
    if( selectionList.isEmpty() )
        return;

    executeCommand(new LockCommand(selectionList, LockCommand::Unlock));
}


void CvsPlugin::update()
{
    kdDebug(8050) << k_funcinfo << endl;

    QStringList selectionList = m_fileView->multipleSelection();
    if( selectionList.isEmpty() )
        return;

    UpdateCommand* cmd = new UpdateCommand(selectionList, m_updateParser);
    cmd->setRecursive(CvsPluginSettings::updateRecursive());
    cmd->setCreateDirectories(CvsPluginSettings::createDirectories());
    cmd->setPruneDirectories(CvsPluginSettings::pruneDirectories());

    executeCommand(cmd);
}


void CvsPlugin::updateToHead()
{
    kdDebug(8050) << k_funcinfo << endl;

    QStringList selectionList = m_fileView->multipleSelection();
    if( selectionList.isEmpty() )
        return;

    UpdateCommand* cmd = new UpdateCommand(selectionList, m_updateParser);
    cmd->setRecursive(CvsPluginSettings::updateRecursive());
    cmd->setCreateDirectories(CvsPluginSettings::createDirectories());
    cmd->setPruneDirectories(CvsPluginSettings::pruneDirectories());
    cmd->setExtraOption("-A");

    executeCommand(cmd);
}


void CvsPlugin::updateToTag()
{
    kdDebug(8050) << k_funcinfo << endl;

    QStringList selectionList = m_fileView->multipleSelection();
    if( selectionList.isEmpty() )
        return;

    UpdateTagCommand* cmd = new UpdateTagCommand(selectionList, m_updateParser);
    cmd->setRecursive(CvsPluginSettings::updateRecursive());

    executeCommand(cmd);
}


void CvsPlugin::createDirectoriesOnUpdate()
{
    bool createDirs = CvsPluginSettings::createDirectories();
    CvsPluginSettings::setCreateDirectories(!createDirs);

    CvsPluginSettings::writeConfig();
}


void CvsPlugin::pruneDirectoriesOnUpdate()
{
    bool pruneDirs = CvsPluginSettings::pruneDirectories();
    CvsPluginSettings::setPruneDirectories(!pruneDirs);

    CvsPluginSettings::writeConfig();
}


void CvsPlugin::commitRecursive()
{
    bool recursive = CvsPluginSettings::commitRecursive();
    CvsPluginSettings::setCommitRecursive(!recursive);

    CvsPluginSettings::writeConfig();
}


void CvsPlugin::updateRecursive()
{
    bool recursive = CvsPluginSettings::updateRecursive();
    CvsPluginSettings::setUpdateRecursive(!recursive);

    CvsPluginSettings::writeConfig();
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

    action = new KAction( i18n("&Difference to Repository (BASE)..."), CTRL+Key_D,
                          this, SLOT( diffToBase() ),
                          actionCollection(), "view_diff_base" );
    hint = i18n("Shows the differences of the selected file to the checked out version (tag BASE)");
    action->setToolTip(hint);
    action->setWhatsThis(hint);

    action = new KAction( i18n("Difference &to Repository (HEAD)..."), CTRL+Key_H,
                          this, SLOT( diffToHead() ),
                          actionCollection(), "view_diff_head" );
    hint = i18n("Shows the differences of the selected file to the newest version in the repository (tag HEAD)");
    action->setToolTip(hint);
    action->setWhatsThis(hint);

    action = new KAction( i18n("Last &Change..."), 0,
                          this, SLOT(showLastChange()),
                          actionCollection(), "view_last_change" );
    hint = i18n("Shows the differences between the last two revisions of the selected file");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    //
    // Advanced Menu
    //
    action = new KAction( i18n("&Tag/Branch..."), 0,
                          this, SLOT( createTag() ),
                          actionCollection(), "create_tag" );
    hint = i18n("Creates a tag or branch for the selected files");
    action->setToolTip(hint);
    action->setWhatsThis(hint);

    action = new KAction( i18n("&Delete Tag..."), 0,
                          this, SLOT( deleteTag() ),
                          actionCollection(), "delete_tag" );
    hint = i18n("Deletes a tag from the selected files");
    action->setToolTip(hint);
    action->setWhatsThis(hint);

    action = new KAction( i18n("&Update to Tag/Date..."), 0,
                          this, SLOT( updateToTag() ),
                          actionCollection(), "update_to_tag" );
    hint = i18n("Updates the selected files to a given tag, branch or date");
    action->setToolTip(hint);
    action->setWhatsThis(hint);

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

    action = new KAction( i18n("&Lock Files"), 0,
                          this, SLOT( lock() ),
                          actionCollection(), "lock_files" );
    hint = i18n("Locks the selected files, so that others cannot modify them");
    action->setToolTip(hint);
    action->setWhatsThis(hint);

    action = new KAction( i18n("Unl&ock Files"), 0,
                          this, SLOT( unlock() ),
                          actionCollection(), "unlock_files" );
    hint = i18n("Unlocks the selected files");
    action->setToolTip(hint);
    action->setWhatsThis(hint);

    //
    // Settings Menu
    //
    KToggleAction* toggleAction = new KToggleAction(
                                i18n("Create &Folders on Update"), 0,
                                this, SLOT( createDirectoriesOnUpdate() ),
                                actionCollection(), "settings_create_dirs" );
    hint = i18n("Determines whether updates create folders");
    toggleAction->setToolTip(hint);
    toggleAction->setWhatsThis(hint);
    toggleAction->setChecked(CvsPluginSettings::createDirectories());

    toggleAction = new KToggleAction( i18n("&Prune Empty Folders on Update"), 0,
                                      this, SLOT( pruneDirectoriesOnUpdate() ),
                                      actionCollection(), "settings_prune_dirs" );
    hint = i18n("Determines whether updates remove empty folders");
    toggleAction->setToolTip(hint);
    toggleAction->setWhatsThis(hint);
    toggleAction->setChecked(CvsPluginSettings::pruneDirectories());

    toggleAction = new KToggleAction( i18n("&Update Recursively"), 0,
                                      this, SLOT( updateRecursive() ),
                                      actionCollection(), "settings_update_recursively" );
    hint = i18n("Determines whether updates are recursive");
    toggleAction->setToolTip(hint);
    toggleAction->setWhatsThis(hint);
    toggleAction->setChecked(CvsPluginSettings::updateRecursive());

    toggleAction = new KToggleAction( i18n("C&ommit && Remove Recursively"), 0,
                                      this, SLOT( commitRecursive() ),
                                      actionCollection(), "settings_commit_recursively" );
    hint = i18n("Determines whether commits and removes are recursive");
    toggleAction->setToolTip(hint);
    toggleAction->setWhatsThis(hint);
    toggleAction->setChecked(CvsPluginSettings::commitRecursive());
}


void CvsPlugin::startService()
{
    kdDebug(8050) << k_funcinfo << "start cvs DCOP service" << endl;

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
