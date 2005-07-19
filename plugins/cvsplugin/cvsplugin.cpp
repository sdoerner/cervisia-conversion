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

#include <commitdlg.h>
#include <cvsjob_stub.h>
#include <cvsservice_stub.h>
#include <repository_stub.h>
#include <selectionintf.h>

#include "cvsjob.h"
#include "addcommand.h"
#include "removecommand.h"
#include "dirignorelist.h"
#include "globalignorelist.h"

#include <kdebug.h>


typedef KGenericFactory<CvsPlugin> CvsPluginFactory;
K_EXPORT_COMPONENT_FACTORY( libcvsplugin, CvsPluginFactory( "cvsplugin" ) )


CvsService_stub* CvsPlugin::m_cvsService = 0;


CvsPlugin::CvsPlugin(QObject* parent, const char* name, const QStringList&)
    : PluginBase(parent, name)
//     , m_cvsService(0)
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


void CvsPlugin::add()
{
    kdDebug(8050) << "CvsPlugin::add()" << endl;

    QStringList selectionList = m_fileView->multipleSelection();
    if( selectionList.isEmpty() )
        return;

    executeCommand(new AddCommand(selectionList, false));
}


void CvsPlugin::addBinary()
{
    kdDebug(8050) << "CvsPlugin::addBinary()" << endl;

    QStringList selectionList = m_fileView->multipleSelection();
    if( selectionList.isEmpty() )
        return;

    executeCommand(new AddCommand(selectionList, true));
}

// TODO: feature commit finished notification
void CvsPlugin::commit()
{
    kdDebug(8050) << "CvsPlugin::commit()" << endl;

    QStringList selectionList = m_fileView->multipleSelection();
    if( selectionList.isEmpty() )
        return;

    // modal dialog
    CommitDialog dlg;
//     dlg.setLogMessage(changelogstr);
//     dlg.setLogHistory(recentCommits);
    dlg.setFileList(selectionList);

    if( dlg.exec() )
    {
        QString msg = dlg.logMessage();
//         if( !recentCommits.contains(msg) )
//         {
//             recentCommits.prepend(msg);
//             while( recentCommits.count() > 50 )
//                 recentCommits.remove(recentCommits.last());
// 
//             KConfig* conf = config();
//             conf->setGroup("CommitLogs");
//             conf->writeEntry(sandbox, recentCommits, COMMIT_SPLIT_CHAR);
//         }

//         DCOPRef jobRef = m_cvsService->commit(selectionList, msg, opt_commitRecursive);
        DCOPRef jobRef = m_cvsService->commit(selectionList, msg, false);
        CvsJob_stub cvsJob(jobRef);

        m_currentJob = new CvsJob(jobRef, CvsJob::Commit);
//         m_currentJob->setRecursive(opt_commitRecursive);
        emit jobPrepared(m_currentJob);

        kdDebug(8050) << "CvsPlugin::commit(): execute cvs job" << endl;
        cvsJob.execute();
    }
}


void CvsPlugin::remove()
{
    kdDebug(8050) << "CvsPlugin::remove()" << endl;

    QStringList selectionList = m_fileView->multipleSelection();
    if( selectionList.isEmpty() )
        return;

    //TODO retrieve 'recursive' from configuration
    executeCommand(new RemoveCommand(selectionList, false/*recursive*/));
}


void CvsPlugin::revert()
{
    kdDebug(8050) << "CvsPlugin::revert()" << endl;

    QStringList selectionList = m_fileView->multipleSelection();
    if( selectionList.isEmpty() )
        return;

//    DCOPRef cvsJob = cvsService->update(list, opt_updateRecursive,
//                                        opt_createDirs, opt_pruneDirs, extraopt);
    DCOPRef jobRef = m_cvsService->update(selectionList, false, false, false, "-C");
    CvsJob_stub cvsJob(jobRef);

    m_currentJob = new CvsJob(jobRef, CvsJob::Update);
//    m_currentJob->setRecursive(opt_updateRecursive);      //TODO: get configuration from part
    emit jobPrepared(m_currentJob);

    kdDebug(8050) << "CvsPlugin::simulateUpdate(): execute cvs job" << endl;
    cvsJob.execute();
}


void CvsPlugin::simulateUpdate()
{
    kdDebug(8050) << "CvsPlugin::simulateUpdate()" << endl;

    QStringList selectionList = m_fileView->multipleSelection();
    if( selectionList.isEmpty() )
        return;

    kdDebug(8050) << "CvsPlugin::simulateUpdate(): add files " << selectionList << endl;

//     DCOPRef cvsJob = cvsService->simulateUpdate(list, opt_updateRecursive,
//             opt_createDirs, opt_pruneDirs);

    DCOPRef jobRef = m_cvsService->simulateUpdate(selectionList, false, false, true);
    CvsJob_stub cvsJob(jobRef);

    m_currentJob = new CvsJob(jobRef, CvsJob::SimulateUpdate);
//    m_currentJob->setRecursive(opt_updateRecursive);      //TODO: get configuration from part
    emit jobPrepared(m_currentJob);

    kdDebug(8050) << "CvsPlugin::simulateUpdate(): execute cvs job" << endl;
    cvsJob.execute();
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
    action->setToolTip( hint );
    action->setWhatsThis( hint );

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
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    action = new KAction( i18n("&Remove From Repository..."), "vcs_remove", Key_Delete,
                          this, SLOT( remove() ),
                          actionCollection(), "file_remove" );
    hint = i18n("Removes the selected files from the repository (cvs remove)");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    action = new KAction( i18n("Rever&t"), 0,
                          this, SLOT( revert() ),
                          actionCollection(), "file_revert_local_changes" );
    hint = i18n("Reverts the selected files (cvs update -C / only cvs 1.11)");
    action->setToolTip( hint );
    action->setWhatsThis( hint );
}


void CvsPlugin::startService()
{
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
    }
}

#include "cvsplugin.moc"
