/*
 *  Copyright (C) 1999-2002 Bernd Gehrmann
 *                          bernd@mail.berlios.de
 *  Copyright (c) 2002-2006 Christian Loose <christian.loose@kdemail.net>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <qlabel.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qpopupmenu.h>
#include <qtextstream.h>
#include <qtooltip.h>
#include <kaboutdata.h>
#include <kaction.h>
#include <kapplication.h>
#include <kfiledialog.h>
#include <kinputdialog.h>
#include <kinstance.h>
#include <klocale.h>
#include <knotifyclient.h>
#include <kprocess.h>
#include <kpropertiesdialog.h>
#include <kstatusbar.h>
#include <kstdaction.h>
#include <kxmlguifactory.h>
#include <krun.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kglobal.h>

#include "progressdlg.h"
#include "diffdlg.h"
#include "resolvedlg.h"
#include "commitdlg.h"
#include "updatedlg.h"
#include "checkoutdlg.h"
#include "tagdlg.h"
#include "mergedlg.h"
#include "historydlg.h"
#include "updateview.h"
#include "updateview_items.h"
#include "protocolview.h"
#include "repositorydlg.h"
#include "settingsdlg.h"
#include "changelogdlg.h"
#include "watchersdlg.h"
#include "cvsinitdlg.h"
#include "misc.h"
#include "cvsservice_stub.h"
#include "repository_stub.h"
//#include "globalignorelist.h"
#include "patchoptiondlg.h"
#include "editwithmenu.h"
#include "pluginmanager.h"
#include "pluginbase.h"
#include "update_parser.h"

#include "cervisiapart.h"
#include "version.h"
#include "cervisiapart.moc"

using Cervisia::TagDialog;

#define COMMIT_SPLIT_CHAR '\r'

K_EXPORT_COMPONENT_FACTORY( libcervisiapart, CervisiaFactory )

CervisiaPart::CervisiaPart( QWidget *parentWidget, const char *widgetName,
                            QObject *parent, const char *name, const QStringList& /*args*/ )
    : KParts::ReadOnlyPart( parent, name )
    , hasRunningJob( false )
    , opt_hideFiles( false )
    , opt_hideUpToDate( false )
    , opt_hideRemoved( false )
    , opt_hideNotInCVS( false )
    , opt_hideEmptyDirectories( false )
    , opt_createDirs( false )
    , opt_pruneDirs( false )
//     , opt_updateRecursive( true )
//     , opt_commitRecursive( true )
    , opt_doCVSEdit( false )
    , recent( 0 )
    , cvsService( 0 )
    , m_statusBar(new KParts::StatusBarExtension(this))
    , m_browserExt( 0 )
    , filterLabel( 0 )
    , m_editWithId(0)
    , m_currentEditMenu(0)
    , m_jobType(Unknown)
    , m_vcsPlugin(0)
{
    KGlobal::locale()->insertCatalogue("cervisia");

    setInstance( CervisiaFactory::instance() );
    m_browserExt = new CervisiaBrowserExtension( this );

    // start the cvs DCOP service
    QString error;
    QCString appId;
    if( KApplication::startServiceByDesktopName("cvsservice", QStringList(), &error, &appId) )
    {
        KMessageBox::sorry(0, i18n("Starting cvsservice failed with message: ") +
            error, "Cervisia");
    }
    else
      // create a reference to the service
      cvsService = new CvsService_stub(appId, "CvsService");

    // Create UI
    KConfig *conf = config();
    conf->setGroup("LookAndFeel");
    bool splitHorz = conf->readBoolEntry("SplitHorizontally",true);

    // When we couldn't start the DCOP service, we just display a QLabel with
    // an explaination
    if( cvsService )
    {
        Orientation o = splitHorz ? QSplitter::Vertical
                                  : QSplitter::Horizontal;
        splitter = new QSplitter(o, parentWidget, widgetName);
        // avoid PartManager's warning that Part's window can't handle focus
        splitter->setFocusPolicy( QWidget::StrongFocus );

        update = new UpdateView(*config(), splitter);
        update->setFocusPolicy( QWidget::StrongFocus );
        update->setFocus();
        connect( update, SIGNAL(contextMenu(KListView*, QListViewItem*, const QPoint&)),
                 this, SLOT(popupRequested(KListView*, QListViewItem*, const QPoint&)) );
        connect( update, SIGNAL(fileOpened(QString)),
                 this, SLOT(openFile(QString)) );

        protocol = new ProtocolView(splitter);
        protocol->setFocusPolicy( QWidget::StrongFocus );

        setWidget(splitter);
    }
    else
        setWidget(new QLabel(i18n("This KPart is non-functional, because the "
                                  "cvs DCOP service could not be started."),
                             parentWidget));

    if( cvsService )
    {
        setupActions();
        readSettings();
        connect( update, SIGNAL( selectionChanged() ), this, SLOT( updateActions() ) );
    }

    setXMLFile( "cervisiaui.rc" );

    QTimer::singleShot(0, this, SLOT(slotSetupStatusBar()));
}

CervisiaPart::~CervisiaPart()
{
    // stop the cvs DCOP service and delete reference
    // plugin's services are quitted by the plugins only the dummy service
    // started in the ctor must be quit here
    if( cvsService && !m_vcsPlugin )
        cvsService->quit();
    delete cvsService;

    if( cvsService )
        writeSettings();
}

KConfig *CervisiaPart::config()
{
    return CervisiaFactory::instance()->config();
}

bool CervisiaPart::openURL( const KURL &u )
{
    // right now, we are unfortunately not network-aware
    if( !u.isLocalFile() )
    {
        KMessageBox::sorry(widget(),
                           i18n("Remote CVS working folders are not "
                                "supported."),
                           "Cervisia");
        return false;
    }

    if( hasRunningJob )
    {
        KMessageBox::sorry(widget(),
                           i18n("You cannot change to a different folder "
                                "while there is a running cvs job."),
                           "Cervisia");
        return false;
    }

    return openSandbox( u.path() );
}


void CervisiaPart::slotSetupStatusBar()
{
    // create the active filter indicator and add it to the statusbar
    filterLabel = new QLabel("UR", m_statusBar->statusBar());
    filterLabel->setFixedSize(filterLabel->sizeHint());
    filterLabel->setText("");
    QToolTip::add(filterLabel,
                  i18n("F - All files are hidden, the tree shows only folders\n"
                       "N - All up-to-date files are hidden\n"
                       "R - All removed files are hidden"));
    m_statusBar->addStatusBarItem(filterLabel, 0, true);
}

void CervisiaPart::setupActions()
{
    KAction *action;
    QString hint;

    actionCollection()->setHighlightingEnabled(true);

    //
    // File Menu
    //
    action = new KAction( i18n("O&pen Sandbox..."), "fileopen", CTRL+Key_O,
                          this, SLOT( slotOpenSandbox() ),
                          actionCollection(), "file_open" );
    hint = i18n("Opens a CVS working folder in the main window");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    recent = new KRecentFilesAction( i18n("Recent Sandboxes"), 0,
                                     this, SLOT( openURL( const KURL & ) ),
                                     actionCollection(), "file_open_recent" );

    action = new KAction( i18n("&Insert ChangeLog Entry..."), 0,
                          this, SLOT( slotChangeLog() ),
                          actionCollection(), "insert_changelog_entry" );
    hint = i18n("Inserts a new intro into the file ChangeLog in the toplevel folder");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

//     action = new KAction( i18n("&Update"), "vcs_update", CTRL+Key_U,
//                           this, SLOT( slotUpdate() ),
//                           actionCollection(), "file_update" );
//     hint = i18n("Updates (cvs update) the selected files and folders");
//     action->setToolTip( hint );
//     action->setWhatsThis( hint );

    action = new KAction( i18n("&Edit"), 0,
                          this, SLOT( slotOpen() ),
                          actionCollection(), "file_edit" );
    hint = i18n("Opens the marked file for editing");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    action = new KAction( i18n("Reso&lve..."), 0,
                          this, SLOT( slotResolve() ),
                          actionCollection(), "file_resolve" );
    hint = i18n("Opens the resolve dialog with the selected file");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    // context menu only
    action = new KAction( i18n("&Properties"), 0,
                          this, SLOT( slotFileProperties() ),
                          actionCollection(), "file_properties" );

    //
    // View Menu
    //
    action = new KAction( i18n("Stop"), "stop", Key_Escape,
                          protocol, SLOT(cancelJob()),
                          actionCollection(), "stop_job" );
    action->setEnabled( false );
    hint = i18n("Stops any running sub-processes");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    action = new KAction( i18n("&History..."), 0,
                          this, SLOT(slotHistory()),
                          actionCollection(), "view_history" );
    hint = i18n("Shows the CVS history as reported by the server");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    action = new KAction( i18n("&Unfold File Tree"), 0,
                          this , SLOT(slotUnfoldTree()),
                          actionCollection(), "view_unfold_tree" );

    hint = i18n("Opens all branches of the file tree");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    action = new KAction( i18n("&Fold File Tree"), 0,
                          this, SLOT(slotFoldTree()),
                          actionCollection(), "view_fold_tree" );
    hint = i18n("Closes all branches of the file tree");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    //
    // Advanced Menu
    //
//     action = new KAction( i18n("&Tag/Branch..."), 0,
//                           this, SLOT(slotCreateTag()),
//                           actionCollection(), "create_tag" );
//     hint = i18n("Creates a tag or branch for the selected files");
//     action->setToolTip( hint );
//     action->setWhatsThis( hint );

//     action = new KAction( i18n("&Delete Tag..."), 0,
//                           this, SLOT(slotDeleteTag()),
//                           actionCollection(), "delete_tag" );
//     hint = i18n("Deletes a tag from the selected files");
//     action->setToolTip( hint );
//     action->setWhatsThis( hint );

//     action = new KAction( i18n("&Update to Tag/Date..."), 0,
//                           this, SLOT(slotUpdateToTag()),
//                           actionCollection(), "update_to_tag" );
//     hint = i18n("Updates the selected files to a given tag, branch or date");
//     action->setToolTip( hint );
//     action->setWhatsThis( hint );

//     action = new KAction( i18n("Update to &HEAD"), 0,
//                           this, SLOT(slotUpdateToHead()),
//                           actionCollection(), "update_to_head" );
//     hint = i18n("Updates the selected files to the HEAD revision");
//     action->setToolTip( hint );
//     action->setWhatsThis( hint );

    action = new KAction( i18n("&Merge..."), 0,
                          this, SLOT(slotMerge()),
                          actionCollection(), "merge" );
    hint = i18n("Merges a branch or a set of modifications into the selected files");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    action = new KAction( i18n("Show &Watchers"), 0,
                          this, SLOT(slotShowWatchers()),
                          actionCollection(), "show_watchers" );
    hint = i18n("Shows the watchers of the selected files");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

//     action = new KAction( i18n("Ed&it Files"), 0,
//                           this, SLOT(slotEdit()),
//                           actionCollection(), "edit_files" );
//     hint = i18n("Edits (cvs edit) the selected files");
//     action->setToolTip( hint );
//     action->setWhatsThis( hint );
// 
//     action = new KAction( i18n("U&nedit Files"), 0,
//                           this, SLOT(slotUnedit()),
//                           actionCollection(), "unedit_files" );
//     hint = i18n("Unedits (cvs unedit) the selected files");
//     action->setToolTip( hint );
//     action->setWhatsThis( hint );

    action = new KAction( i18n("Show &Editors"), 0,
                          this, SLOT(slotShowEditors()),
                          actionCollection(), "show_editors" );
    hint = i18n("Shows the editors of the selected files");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

//     action = new KAction( i18n("&Lock Files"), 0,
//                           this, SLOT(slotLock()),
//                           actionCollection(), "lock_files" );
//     hint = i18n("Locks the selected files, so that others cannot modify them");
//     action->setToolTip( hint );
//     action->setWhatsThis( hint );

//     action = new KAction( i18n("Unl&ock Files"), 0,
//                           this, SLOT(slotUnlock()),
//                           actionCollection(), "unlock_files" );
//     hint = i18n("Unlocks the selected files");
//     action->setToolTip( hint );
//     action->setWhatsThis( hint );

    action = new KAction( i18n("Create &Patch Against Repository..."), 0,
                          this, SLOT(slotMakePatch()),
                          actionCollection(), "make_patch" );
    hint = i18n("Creates a patch from the modifications in your sandbox");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    //
    // Repository Menu
    //
    action = new KAction( i18n("&Create..."), 0,
                          this, SLOT(slotCreateRepository()),
                          actionCollection(), "repository_create" );

    action = new KAction( i18n("&Checkout..."), 0,
                          this, SLOT(slotCheckout()),
                          actionCollection(), "repository_checkout" );
    hint = i18n("Allows you to checkout a module from a repository");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    action = new KAction( i18n("&Import..."), 0,
                          this, SLOT(slotImport()),
                          actionCollection(), "repository_import" );
    hint = i18n("Allows you to import a module into a repository");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    action = new KAction( i18n("&Repositories..."), 0,
                          this, SLOT(slotRepositories()),
                          actionCollection(), "show_repositories" );
    hint = i18n("Configures a list of repositories you regularly use");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    //
    // Settings menu
    //
    KToggleAction* toggaction = new KToggleAction( i18n("Hide All &Files"), 0,
                                this, SLOT(slotHideFiles()),
                                actionCollection(), "settings_hide_files" );
    toggaction->setCheckedState(i18n("Show All &Files"));
    hint = i18n("Determines whether only folders are shown");
    toggaction->setToolTip( hint );
    toggaction->setWhatsThis( hint );

    toggaction = new KToggleAction( i18n("Hide Unmodified Files"), 0,
                                this, SLOT(slotHideUpToDate()),
                                actionCollection(), "settings_hide_uptodate" );
    toggaction->setCheckedState(i18n("Show Unmodified Files"));
    hint = i18n("Determines whether files with status up-to-date or "
                "unknown are hidden");
    toggaction->setToolTip( hint );
    toggaction->setWhatsThis( hint );

    toggaction = new KToggleAction( i18n("Hide Removed Files"), 0,
                                this, SLOT(slotHideRemoved()),
                                actionCollection(), "settings_hide_removed" );
    toggaction->setCheckedState(i18n("Show Removed Files"));
    hint = i18n("Determines whether removed files are hidden");
    toggaction->setToolTip( hint );
    toggaction->setWhatsThis( hint );

    toggaction = new KToggleAction( i18n("Hide Non-CVS Files"), 0,
                                this, SLOT(slotHideNotInCVS()),
                                actionCollection(), "settings_hide_notincvs" );
    toggaction->setCheckedState(i18n("Show Non-CVS Files"));
    hint = i18n("Determines whether files not in CVS are hidden");
    toggaction->setToolTip( hint );
    toggaction->setWhatsThis( hint );

    toggaction = new KToggleAction( i18n("Hide Empty Folders"), 0,
                                    this, SLOT(slotHideEmptyDirectories()),
                                    actionCollection(), "settings_hide_empty_directories" );
    toggaction->setCheckedState(i18n("Show Empty Folders"));
    hint = i18n("Determines whether folders without visible entries are hidden");
    toggaction->setToolTip( hint );
    toggaction->setWhatsThis( hint );

    action = new KToggleAction( i18n("Create &Folders on Update"), 0,
                                this, SLOT(slotCreateDirs()),
                                actionCollection(), "settings_create_dirs" );
    hint = i18n("Determines whether updates create folders");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    action = new KToggleAction( i18n("&Prune Empty Folders on Update"), 0,
                                this, SLOT(slotPruneDirs()),
                                actionCollection(), "settings_prune_dirs" );
    hint = i18n("Determines whether updates remove empty folders");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

//     action = new KToggleAction( i18n("&Update Recursively"), 0,
//                                 this, SLOT(slotUpdateRecursive()),
//                                 actionCollection(), "settings_update_recursively" );
//     hint = i18n("Determines whether updates are recursive");
//     action->setToolTip( hint );
//     action->setWhatsThis( hint );

//     action = new KToggleAction( i18n("C&ommit && Remove Recursively"), 0,
//                                 this, SLOT(slotCommitRecursive()),
//                                 actionCollection(), "settings_commit_recursively" );
//     hint = i18n("Determines whether commits and removes are recursive");
//     action->setToolTip( hint );
//     action->setWhatsThis( hint );

    action = new KToggleAction( i18n("Do cvs &edit Automatically When Necessary"), 0,
                                this, SLOT(slotDoCVSEdit()),
                                actionCollection(), "settings_do_cvs_edit" );
    hint = i18n("Determines whether automatic cvs editing is active");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    action = new KAction( i18n("Configure Cervisia..."), "configure", 0,
                          this, SLOT(slotConfigure()),
                          actionCollection(), "configure_cervisia" );
    hint = i18n("Allows you to configure the Cervisia KPart");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    //
    // Help Menu
    //
    action = KStdAction::help( this, SLOT(slotHelp()),
                               actionCollection() );

    action = new KAction( i18n("CVS &Manual"), 0,
                          this, SLOT(slotCVSInfo()),
                          actionCollection(), "help_cvs_manual" );
    hint = i18n("Opens the help browser with the CVS documentation");
    action->setToolTip( hint );
    action->setWhatsThis( hint );

    //
    // Folder context menu
    //
    toggaction = new KToggleAction( i18n("Unfold Folder"), 0,
                                    this, SLOT( slotUnfoldFolder() ),
                                    actionCollection(), "unfold_folder" );
    toggaction->setCheckedState(i18n("Fold Folder"));

    //action = KStdAction::aboutApp( this, SLOT(aboutCervisia()),
    //               actionCollection(), "help_about_cervisia" );
}


void CervisiaPart::popupRequested(KListView*, QListViewItem* item, const QPoint& p)
{
    QString xmlName = "context_popup";

    if( isDirItem(item) && update->fileSelection().isEmpty() )
    {
        xmlName = "folder_context_popup";
        KToggleAction* action = static_cast<KToggleAction*>(actionCollection()->action("unfold_folder"));
        action->setChecked(item->isOpen());
    }

    if( QPopupMenu* popup = static_cast<QPopupMenu*>(hostContainer(xmlName)) )
    {
        if( isFileItem(item) )
        {
            // remove old 'Edit with...' menu
            if( m_editWithId && popup->findItem(m_editWithId) != 0 )
            {
                popup->removeItem(m_editWithId);
                delete m_currentEditMenu; 

                m_editWithId      = 0;
                m_currentEditMenu = 0;
            }

            // get name of selected file
            QString selectedFile;
            update->getSingleSelection(&selectedFile);

            if( !selectedFile.isEmpty() )
            {
                KURL u;
                u.setPath(sandbox + "/" + selectedFile);

                m_currentEditMenu = new Cervisia::EditWithMenu(u, popup);

                if( m_currentEditMenu->menu() )
                    m_editWithId = popup->insertItem(i18n("Edit With"), 
                                              m_currentEditMenu->menu(), -1, 1);
            }
        }

        popup->exec(p);
    }
    else
        kdDebug(8050) << "CervisiaPart: can't get XML definition for " << xmlName << ", factory()=" << factory() << endl;
}

void CervisiaPart::updateActions()
{
    bool hassandbox = !sandbox.isEmpty();
    stateChanged("has_sandbox", hassandbox ? StateNoReverse : StateReverse);

    bool single = update->hasSingleSelection();
    stateChanged("has_single_selection", single ? StateNoReverse
                                                : StateReverse);

    bool singleFolder = (update->multipleSelection().count() == 1);
    stateChanged("has_single_folder", singleFolder ? StateNoReverse
                                                   : StateReverse);

    m_browserExt->setPropertiesActionEnabled(single);

    //    bool nojob = !( actionCollection()->action( "stop_job" )->isEnabled() );
    bool selected = (update->currentItem() != 0);
    bool nojob = !hasRunningJob && selected;

    stateChanged("item_selected", selected ? StateNoReverse : StateReverse);
    stateChanged("has_no_job", nojob ? StateNoReverse : StateReverse);
    stateChanged("has_running_job", hasRunningJob ? StateNoReverse
                                                  : StateReverse);

}


void CervisiaPart::aboutCervisia()
{
    QString aboutstr(i18n("Cervisia %1\n"
                          "(Using KDE %2)\n"
                          "\n"
                          "Copyright (c) 1999-2002\n"
                          "Bernd Gehrmann <bernd@mail.berlios.de>\n"
                          "\n"
                          "This program is free software; you can redistribute it and/or modify\n"
                          "it under the terms of the GNU General Public License as published by\n"
                          "the Free Software Foundation; either version 2 of the License, or\n"
                          "(at your option) any later version.\n"
                          "This program is distributed in the hope that it will be useful,\n"
                          "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
                          "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
                          "GNU General Public License for more details.\n"
                          "See the ChangeLog file for a list of contributors."));
    QMessageBox::about(0, i18n("About Cervisia"),
                       aboutstr.arg(CERVISIA_VERSION).arg(KDE_VERSION_STRING));
}


KAboutData* CervisiaPart::createAboutData()
{
    KAboutData* about = new KAboutData(
                            "cervisiapart", I18N_NOOP("Cervisia Part"),
                            CERVISIA_VERSION, I18N_NOOP("A CVS frontend"),
                            KAboutData::License_GPL,
                            I18N_NOOP("Copyright (c) 1999-2002 Bernd Gehrmann"), 0,
                            "http://www.kde.org/apps/cervisia");

    about->addAuthor("Bernd Gehrmann", I18N_NOOP("Original author and former "
                    "maintainer"), "bernd@mail.berlios.de", 0);
    about->addAuthor("Christian Loose", I18N_NOOP("Maintainer"),
                    "christian.loose@hamburg.de", 0);
    about->addAuthor("Andr\303\251 W\303\266bbeking", I18N_NOOP("Developer"),
                    "woebbeking@web.de", 0);

    about->addCredit("Richard Moore", I18N_NOOP("Conversion to KPart"),
                    "rich@kde.org", 0);

    return about;
}


void CervisiaPart::slotOpenSandbox()
{
    QString dirname = KFileDialog::getExistingDirectory(":CervisiaPart", widget(),
                                                        i18n("Open Sandbox"));
    if (dirname.isEmpty())
        return;

    openSandbox(dirname);
}


void CervisiaPart::slotChangeLog()
{
    // Modal dialog
    ChangeLogDialog dlg(*config(), widget());
    if (dlg.readFile(sandbox + "/ChangeLog"))
    {
        if (dlg.exec())
            changelogstr = dlg.message();
    }
}


void CervisiaPart::slotOpen()
{
    QStringList filenames = update->fileSelection();
    if (filenames.isEmpty())
        return;
    openFiles(filenames);
}


void CervisiaPart::openFile(QString filename)
{
    QStringList files;
    files << filename;
    openFiles(files);
}


void CervisiaPart::openFiles(const QStringList &filenames)
{
//     // call cvs edit automatically?
//     if( opt_doCVSEdit )
//     {
//         QStringList files;
// 
//         // only edit read-only files
//         QStringList::ConstIterator it  = filenames.begin();
//         QStringList::ConstIterator end = filenames.end();
//         for( ; it != end; ++it )
//         {
//             if( !QFileInfo(*it).isWritable() )
//                 files << *it;
//         }
// 
//         if( files.count() )
//         {
//             DCOPRef job = cvsService->edit(files);
// 
//             ProgressDialog dlg(widget(), "Edit", job, "edit", i18n("CVS Edit"));
//             if( !dlg.execute() )
//                 return;
//         }
//     }

    // Now open the files by using KRun
    QDir dir(sandbox);

    QStringList::ConstIterator it  = filenames.begin();
    QStringList::ConstIterator end = filenames.end();
    for( ; it != end; ++it )
    {
        KURL u;
        u.setPath(dir.absFilePath(*it));
        KRun* run = new KRun(u, 0, true, false);
        run->setRunExecutables(false);
    }
}


void CervisiaPart::slotResolve()
{
    QString filename;
    update->getSingleSelection(&filename);
    if (filename.isEmpty())
        return;

    // Non-modal dialog
    ResolveDialog *l = new ResolveDialog(*config());
    if (l->parseFile(filename))
        l->show();
    else
        delete l;
}


void CervisiaPart::slotMerge()
{
    MergeDialog dlg(cvsService, widget());

    if (dlg.exec())
    {
        QString tagopt;
        if (dlg.byBranch())
        {
            tagopt = "-j ";
            tagopt += dlg.branch();
        }
        else
        {
            tagopt = "-j ";
            tagopt += dlg.tag1();
            tagopt += " -j ";
            tagopt += dlg.tag2();
        }
        tagopt += " ";
        updateSandbox(tagopt);
    }
}


void CervisiaPart::slotFileProperties()
{
    QString filename;
    update->getSingleSelection(&filename);
    if( filename.isEmpty() )
        return;

    // Create URL from selected filename
    QDir dir(sandbox);

    KURL u;
    u.setPath(dir.absFilePath(filename));

    // show file properties dialog
    (void)new KPropertiesDialog(u);
}


void CervisiaPart::updateSandbox(const QString &extraopt)
{
    QStringList list = update->multipleSelection();
    if (list.isEmpty())
        return;

//     update->prepareJob(opt_updateRecursive, UpdateView::Update);

    DCOPRef cvsJob = cvsService->update(list, opt_updateRecursive,
                        opt_createDirs, opt_pruneDirs, extraopt);

    // get command line from cvs job
    QString cmdline;
    DCOPReply reply = cvsJob.call("cvsCommand()");
    if( reply.isValid() )
        reply.get<QString>(cmdline);

    if( protocol->startJob(true) )
    {
        showJobStart(cmdline);
        connect( protocol, SIGNAL(receivedLine(QString)), update, SLOT(processUpdateLine(QString)) );
        connect( protocol, SIGNAL(jobFinished(bool, int)), update, SLOT(finishJob(bool, int)) );
        connect( protocol, SIGNAL(jobFinished(bool, int)), this, SLOT(slotJobFinished()) );
    }
}

void CervisiaPart::slotShowWatchers()
{
    QStringList list = update->multipleSelection();
    if (list.isEmpty())
        return;

    // Non-modal dialog
    WatchersDialog* dlg = new WatchersDialog(*config());
    if( dlg->parseWatchers(cvsService, list) )
        dlg->show();
    else
        delete dlg;
}


// void CervisiaPart::slotEdit()
// {
//     QStringList list = update->multipleSelection();
//     if (list.isEmpty())
//         return;
// 
//     DCOPRef cvsJob = cvsService->edit(list);
// 
//     // get command line from cvs job
//     QString cmdline = cvsJob.call("cvsCommand()");
// 
//     if( protocol->startJob() )
//     {
//         showJobStart(cmdline);
//         connect( protocol, SIGNAL(jobFinished(bool, int)),
//                  this,     SLOT(slotJobFinished()) );
//     }
// }


// void CervisiaPart::slotUnedit()
// {
//     QStringList list = update->multipleSelection();
//     if (list.isEmpty())
//         return;
// 
//     DCOPRef cvsJob = cvsService->unedit(list);
// 
//     // get command line from cvs job
//     QString cmdline = cvsJob.call("cvsCommand()");
// 
//     if( protocol->startJob() )
//     {
//         showJobStart(cmdline);
//         connect( protocol, SIGNAL(jobFinished(bool, int)),
//                  this,     SLOT(slotJobFinished()) );
//     }
// }


// void CervisiaPart::slotLock()
// {
//     QStringList list = update->multipleSelection();
//     if (list.isEmpty())
//         return;
// 
//     DCOPRef cvsJob = cvsService->lock(list);
// 
//     // get command line from cvs job
//     QString cmdline = cvsJob.call("cvsCommand()");
// 
//     if( protocol->startJob() )
//     {
//         showJobStart(cmdline);
//         connect( protocol, SIGNAL(jobFinished(bool, int)),
//                  this,     SLOT(slotJobFinished()) );
//     }
// }


// void CervisiaPart::slotUnlock()
// {
//     QStringList list = update->multipleSelection();
//     if (list.isEmpty())
//         return;
// 
//     DCOPRef cvsJob = cvsService->unlock(list);
// 
//     // get command line from cvs job
//     QString cmdline = cvsJob.call("cvsCommand()");
// 
//     if( protocol->startJob() )
//     {
//         showJobStart(cmdline);
//         connect( protocol, SIGNAL(jobFinished(bool, int)),
//                  this,     SLOT(slotJobFinished()) );
//     }
// }


void CervisiaPart::slotShowEditors()
{
    QStringList list = update->multipleSelection();
    if (list.isEmpty())
        return;

    DCOPRef cvsJob = cvsService->editors(list);

    // get command line from cvs job
    QString cmdline = cvsJob.call("cvsCommand()");

    if( protocol->startJob() )
    {
        showJobStart(cmdline);
        connect( protocol, SIGNAL(jobFinished(bool, int)),
                 this,     SLOT(slotJobFinished()) );
    }
}


void CervisiaPart::slotMakePatch()
{
/*    Cervisia::PatchOptionDialog optionDlg;
    if( optionDlg.exec() == KDialogBase::Rejected )
        return;
    
    QString format      = optionDlg.formatOption();
    QString diffOptions = optionDlg.diffOptions();

    DCOPRef job = cvsService->makePatch(diffOptions, format);
    if( !cvsService->ok() )
        return;

    ProgressDialog dlg(widget(), "Diff", job, "", i18n("CVS Diff"));
    if( !dlg.execute() )
        return;

    QString fileName = KFileDialog::getSaveFileName();
    if( fileName.isEmpty() )
        return;

    if( !Cervisia::CheckOverwrite(fileName) )
        return;

    QFile f(fileName);
    if( !f.open(IO_WriteOnly) )
    {
        KMessageBox::sorry(widget(),
                           i18n("Could not open file for writing."),
                           "Cervisia");
        return;
    }

    QTextStream t(&f);
    QString line;
    while( dlg.getLine(line) )
        t << line << '\n';

    f.close();*/
}


void CervisiaPart::slotImport()
{
//     CheckoutDialog dlg(*config(), cvsService, CheckoutDialog::Import, widget());
// 
//     if( !dlg.exec() )
//         return;
// 
//     DCOPRef cvsJob = cvsService->import(dlg.workingDirectory(), dlg.repository(),
//                                         dlg.module(), dlg.ignoreFiles(),
//                                         dlg.comment(), dlg.vendorTag(),
//                                         dlg.releaseTag(), dlg.importBinary(),
//                                         dlg.useModificationTime());
// 
//     // get command line from cvs job
//     QString cmdline = cvsJob.call("cvsCommand()");
// 
//     if( protocol->startJob() )
//     {
//         showJobStart(cmdline);
//         connect( protocol, SIGNAL(jobFinished(bool, int)),
//                  this,     SLOT(slotJobFinished()) );
//     }
}


void CervisiaPart::slotCreateRepository()
{
    Cervisia::CvsInitDialog dlg(widget());

    if( !dlg.exec() )
        return;

    DCOPRef cvsJob = cvsService->createRepository(dlg.directory());

    QString cmdline = cvsJob.call("cvsCommand()");

    if( protocol->startJob() )
    {
        showJobStart(cmdline);
        connect( protocol, SIGNAL(jobFinished(bool, int)),
                 this,     SLOT(slotJobFinished()) );
    }
}


void CervisiaPart::slotCheckout()
{
    CheckoutDialog dlg(widget());

    // add widgets from plugins to checkout dialog
    Cervisia::PluginList plugins = Cervisia::PluginManager::self()->plugins();

    Cervisia::PluginList::ConstIterator it;
    for( it = plugins.begin(); it != plugins.end(); ++it )
    {
        Cervisia::CheckoutWidgetBase* w = (*it)->checkoutWidget(&dlg);
        if( w )
            dlg.addCheckoutWidget((*it)->type(), w);
    }

    // did the user cancel the dialog?
    if( !dlg.exec() )
        return;

    // change plugin for checkout
    QString pluginType = dlg.pluginType();
    Cervisia::PluginBase* plugin = Cervisia::PluginManager::self()->pluginForType(pluginType);
    protocol->updatePlugin(plugin);

    disconnect(plugin, SIGNAL(commandPrepared(Cervisia::CommandBase*)),
               this, 0);

    connect(plugin, SIGNAL(commandPrepared(Cervisia::CommandBase*)),
            this, SLOT(commandPrepared(Cervisia::CommandBase*)));

    // checkout the working copy
    plugin->checkout(dlg.currentWidget());
}


void CervisiaPart::slotRepositories()
{
    RepositoryDialog *l = new RepositoryDialog(*config(), cvsService, widget());
    l->show();
}


// void CervisiaPart::slotCreateTag()
// {
//     createOrDeleteTag(TagDialog::Create);
// }


// void CervisiaPart::slotDeleteTag()
// {
//     createOrDeleteTag(TagDialog::Delete);
// }


// void CervisiaPart::createOrDeleteTag(TagDialog::ActionType action)
// {
//     QStringList list = update->multipleSelection();
//     if (list.isEmpty())
//         return;
// 
//     TagDialog dlg(action, cvsService, widget());
// 
//     if (dlg.exec())
//     {
//         DCOPRef cvsJob;
// 
//         if( action == TagDialog::Create )
//             cvsJob = cvsService->createTag(list, dlg.tag(), dlg.branchTag(),
//                                            dlg.forceTag());
//         else
//             cvsJob = cvsService->deleteTag(list, dlg.tag(), dlg.branchTag(),
//                                            dlg.forceTag());
// 
//         // get command line from cvs job
//         QString cmdline = cvsJob.call("cvsCommand()");
// 
//         if( protocol->startJob() )
//         {
//             showJobStart(cmdline);
//             connect( protocol, SIGNAL(jobFinished(bool, int)),
//                      this,     SLOT(slotJobFinished()) );
//         }
//     }
// }



void CervisiaPart::slotHistory()
{
    // Non-modal dialog
    HistoryDialog *l = new HistoryDialog(*config());
    if (l->parseHistory(cvsService))
        l->show();
    else
        delete l;
}


void CervisiaPart::slotHideFiles()
{
    opt_hideFiles = !opt_hideFiles;
    setFilter();
}


void CervisiaPart::slotHideUpToDate()
{
    opt_hideUpToDate = !opt_hideUpToDate;
    setFilter();
}


void CervisiaPart::slotHideRemoved()
{
    opt_hideRemoved = !opt_hideRemoved;
    setFilter();
}


void CervisiaPart::slotHideNotInCVS()
{
    opt_hideNotInCVS = !opt_hideNotInCVS;
    setFilter();
}


void CervisiaPart::slotHideEmptyDirectories()
{
    opt_hideEmptyDirectories = !opt_hideEmptyDirectories;
    setFilter();
}


void CervisiaPart::slotFoldTree()
{
    update->foldTree();
    setFilter();
}

void CervisiaPart::slotUnfoldTree()
{
    update->unfoldTree();
    setFilter();
}


void CervisiaPart::slotUnfoldFolder()
{
    update->unfoldSelectedFolders();
    setFilter();
}


void CervisiaPart::slotCreateDirs()
{
    opt_createDirs = !opt_createDirs;
}


void CervisiaPart::slotPruneDirs()
{
    opt_pruneDirs = !opt_pruneDirs;
}


// void CervisiaPart::slotUpdateRecursive()
// {
//     opt_updateRecursive = !opt_updateRecursive;
// }


// void CervisiaPart::slotCommitRecursive()
// {
//     opt_commitRecursive = !opt_commitRecursive;
// }


void CervisiaPart::slotDoCVSEdit()
{
    opt_doCVSEdit = !opt_doCVSEdit;
}

void CervisiaPart::slotConfigure()
{
    KConfig *conf = config();
    SettingsDialog *l = new SettingsDialog( conf, widget() );
    l->exec();

    conf->setGroup("LookAndFeel");
    bool splitHorz = conf->readBoolEntry("SplitHorizontally",true);
    splitter->setOrientation( splitHorz ?
                              QSplitter::Vertical :
                              QSplitter::Horizontal);
}

void CervisiaPart::slotHelp()
{
    emit setStatusBarText( i18n("Invoking help on Cervisia") );
    KApplication::startServiceByDesktopName("khelpcenter", QString("help:/cervisia/index.html"));
}


void CervisiaPart::slotCVSInfo()
{
    emit setStatusBarText( i18n("Invoking help on CVS") );
    KApplication::startServiceByDesktopName("khelpcenter", QString("info:/cvs/Top"));
}


void CervisiaPart::showJobStart(const QString &cmdline)
{
//     hasRunningJob = true;
//     actionCollection()->action( "stop_job" )->setEnabled( true );
// 
//     emit setStatusBarText( cmdline );
//     updateActions();
}


void CervisiaPart::slotJobFinished()
{
    actionCollection()->action("stop_job")->setEnabled(false);
    hasRunningJob = false;
    emit setStatusBarText(i18n("Done"));
    updateActions();

    // restore plugin
    protocol->updatePlugin(m_vcsPlugin);
    disconnect(m_vcsPlugin, SIGNAL(commandPrepared(Cervisia::CommandBase*)),
               this, 0);
    connect(m_vcsPlugin, SIGNAL(commandPrepared(Cervisia::CommandBase*)),
            this, SLOT(commandPrepared(Cervisia::CommandBase*)));

//     disconnect( protocol, SIGNAL(receivedLine(QString)),
//                 update,   SLOT(processUpdateLine(QString)) );

    if( m_jobType == Commit )
    {
        KNotifyClient::event(widget()->parentWidget()->winId(), "cvs_commit_done",
                             i18n("A CVS commit to repository %1 is done")
                             .arg(repository));
        m_jobType = Unknown;
    }
}


void CervisiaPart::commandPrepared(Cervisia::CommandBase* cmd)
{
    kdDebug(8050) << k_funcinfo << endl;

    hasRunningJob = true;
    actionCollection()->action("stop_job")->setEnabled(true);

    emit setStatusBarText(cmd->commandString());
    updateActions();

    m_jobType = Unknown;
    if( cmd->action() == Cervisia::CommandBase::Commit )
        m_jobType = Commit;

    // connect to the command signal
    connect(cmd, SIGNAL(jobExited(bool, int)),
            this, SLOT(slotJobFinished()));
}


bool CervisiaPart::openSandbox(const QString &dirname)
{
    // Do we have a cvs service?
    if( !cvsService )
        return false;

    kdDebug(8050) << k_funcinfo << "dirname = " << dirname << endl;

    // plugin's services are quitted by the plugins only the dummy service
    // started in the ctor must be quit here
    if( cvsService && !m_vcsPlugin )
        cvsService->quit();

    KURL url;
    url.setPath(dirname);
    m_vcsPlugin = Cervisia::PluginManager::self()->pluginForUrl(url);

//     Repository_stub cvsRepository(cvsService->app(), "CvsRepository");

//     // change the working copy directory for the cvs DCOP service
//     bool opened = cvsRepository.setWorkingCopy(dirname);

//     if( !cvsRepository.ok() || !opened )
    if( !m_vcsPlugin )
    {
        KMessageBox::sorry(widget(),
                           i18n("This is not a CVS folder.\n"
                           "If you did not intend to use Cervisia, you can "
                           "switch view modes within Konqueror."),
                           "Cervisia");

        // remove path from recent sandbox menu
        QFileInfo fi(dirname);
        recent->removeURL( KURL::fromPathOrURL(fi.absFilePath()) );

        return false;
    }

    changelogstr = "";
//    sandbox      = "";
//    repository   = "";

    //FIXME: temporarily get cvsservice reference from plugin
    delete cvsService;
    cvsService = new CvsService_stub(m_vcsPlugin->service());
    protocol->updatePlugin(m_vcsPlugin);

    m_vcsPlugin->setFileView(update);
    Cervisia::UpdateParser* parser = m_vcsPlugin->updateParser();

    // make sure we don't connect to the signal twice
    disconnect(m_vcsPlugin, SIGNAL(commandPrepared(Cervisia::CommandBase*)),
               update, 0);
    disconnect(parser, SIGNAL(updateItemStatus(const QString&, Cervisia::EntryStatus, bool)),
               update, 0);
    disconnect(m_vcsPlugin, SIGNAL(commandPrepared(Cervisia::CommandBase*)),
               this, 0);

    connect(m_vcsPlugin, SIGNAL(commandPrepared(Cervisia::CommandBase*)),
            update, SLOT(commandPrepared(Cervisia::CommandBase*)));
    connect(m_vcsPlugin, SIGNAL(commandPrepared(Cervisia::CommandBase*)),
            this, SLOT(commandPrepared(Cervisia::CommandBase*)));
    connect(parser, SIGNAL(updateItemStatus(const QString&, Cervisia::EntryStatus, bool)),
            update, SLOT(updateItem(const QString&, Cervisia::EntryStatus, bool)));

//    connect(m_vcsPlugin, SIGNAL(jobExited(bool, int)),
//            update, SLOT(finishJob(bool, int)));
//    connect(m_vcsPlugin, SIGNAL(jobExited(bool, int)),
//            this, SLOT(slotJobFinished()));

    // get path of sandbox for recent sandbox menu
    KURL wc = m_vcsPlugin->workingCopy();
    sandbox = wc.path();
    recent->addURL(wc);
//     sandbox = cvsRepository.workingCopy();
//     recent->addURL( KURL::fromPathOrURL(sandbox) );

    // get repository for the caption of the window
    repository = m_vcsPlugin->repository();
//     repository = cvsRepository.location();
    emit setWindowCaption(sandbox + "(" + repository + ")");

    // set m_url member for tabbed window modus of Konqueror
    m_url = KURL::fromPathOrURL(sandbox);

//     // *NOTICE*
//     // The order is important here. We have to set the m_url member before
//     // calling this function because the progress dialog uses the enter_loop()/
//     // exit_loop() methods. Those methods result in a call to queryExit() in
//     // cervisiashell.cpp which then uses the m_url member to save the last used
//     // directory.
//     if( cvsRepository.retrieveCvsignoreFile() )
//         Cervisia::GlobalIgnoreList().retrieveServerIgnoreList(cvsService,
//                                                               repository);

    QDir::setCurrent(sandbox);
    update->openDirectory(sandbox);
    setFilter();

    KConfig *conf = config();
    conf->setGroup("General");
    bool dostatus = conf->readBoolEntry(repository.contains(":")?
                                        "StatusForRemoteRepos" :
                                        "StatusForLocalRepos",
                                        false);
    if (dostatus)
    {
        update->setSelected(update->firstChild(), true);
        m_vcsPlugin->simulateUpdate();
    }

    //load the recentCommits for this app from the KConfig app
    conf->setGroup( "CommitLogs" );
    recentCommits = conf->readListEntry( sandbox, COMMIT_SPLIT_CHAR );

    return true;
}


void CervisiaPart::setFilter()
{
    UpdateView::Filter filter = UpdateView::Filter(0);
    if (opt_hideFiles)
        filter = UpdateView::Filter(filter | UpdateView::OnlyDirectories);
    if (opt_hideUpToDate)
        filter = UpdateView::Filter(filter | UpdateView::NoUpToDate);
    if (opt_hideRemoved)
        filter = UpdateView::Filter(filter | UpdateView::NoRemoved);
    if (opt_hideNotInCVS)
        filter = UpdateView::Filter(filter | UpdateView::NoNotInCVS);
    if (opt_hideEmptyDirectories)
        filter = UpdateView::Filter(filter | UpdateView::NoEmptyDirectories);
    update->setFilter(filter);

    QString str;
    if (opt_hideFiles)
        str = "F";
    else
        {
            if (opt_hideUpToDate)
                str += "N";
            if (opt_hideRemoved)
                str += "R";
        }

    if( filterLabel )
        filterLabel->setText(str);
}


void CervisiaPart::readSettings()
{
    KConfig* config = CervisiaFactory::instance()->config();

    config->setGroup("Session");
    recent->loadEntries( config );

    // Unfortunately, the KConfig systems sucks and we have to live
    // with all entries in one group for session management.

    opt_createDirs = config->readBoolEntry("Create Dirs", true);
    (static_cast<KToggleAction *> (actionCollection()->action( "settings_create_dirs" )))
    ->setChecked( opt_createDirs );

    opt_pruneDirs = config->readBoolEntry("Prune Dirs", true);
    (static_cast<KToggleAction *> (actionCollection()->action( "settings_prune_dirs" )))
    ->setChecked( opt_pruneDirs );

//     opt_updateRecursive = config->readBoolEntry("Update Recursive", false);
//     (static_cast<KToggleAction *> (actionCollection()->action( "settings_update_recursively" )))
//     ->setChecked( opt_updateRecursive );

//     opt_commitRecursive = config->readBoolEntry("Commit Recursive", false);
//     (static_cast<KToggleAction *> (actionCollection()->action( "settings_commit_recursively" )))
//     ->setChecked( opt_commitRecursive );

    opt_doCVSEdit = config->readBoolEntry("Do cvs edit", false);
    (static_cast<KToggleAction *> (actionCollection()->action( "settings_do_cvs_edit" )))
    ->setChecked( opt_doCVSEdit );

    opt_hideFiles = config->readBoolEntry("Hide Files", false);
    (static_cast<KToggleAction *> (actionCollection()->action( "settings_hide_files" )))
    ->setChecked( opt_hideFiles );

    opt_hideUpToDate = config->readBoolEntry("Hide UpToDate Files", false);
    (static_cast<KToggleAction *> (actionCollection()->action( "settings_hide_uptodate" )))
    ->setChecked( opt_hideUpToDate );

    opt_hideRemoved = config->readBoolEntry("Hide Removed Files", false);
    (static_cast<KToggleAction *> (actionCollection()->action( "settings_hide_removed" )))
    ->setChecked( opt_hideRemoved );

    opt_hideNotInCVS = config->readBoolEntry("Hide Non CVS Files", false);
    (static_cast<KToggleAction *> (actionCollection()->action( "settings_hide_notincvs" )))
    ->setChecked( opt_hideNotInCVS );

    opt_hideEmptyDirectories = config->readBoolEntry("Hide Empty Directories", false);
    (static_cast<KToggleAction *> (actionCollection()->action( "settings_hide_empty_directories" )))
    ->setChecked( opt_hideEmptyDirectories );

    setFilter();

    int splitterpos1 = config->readNumEntry("Splitter Pos 1", 0);
    int splitterpos2 = config->readNumEntry("Splitter Pos 2", 0);
    if (splitterpos1)
    {
        QValueList<int> sizes;
        sizes << splitterpos1;
        sizes << splitterpos2;
        splitter->setSizes(sizes);
    }
}


void CervisiaPart::writeSettings()
{
    KConfig* config = CervisiaFactory::instance()->config();

    config->setGroup("Session");
    recent->saveEntries( config );

    config->writeEntry("Create Dirs", opt_createDirs);
    config->writeEntry("Prune Dirs", opt_pruneDirs);
//     config->writeEntry("Update Recursive", opt_updateRecursive);
//     config->writeEntry("Commit Recursive", opt_commitRecursive);
    config->writeEntry("Do cvs edit", opt_doCVSEdit);
    config->writeEntry("Hide Files", opt_hideFiles);
    config->writeEntry("Hide UpToDate Files", opt_hideUpToDate);
    config->writeEntry("Hide Removed Files", opt_hideRemoved);
    config->writeEntry("Hide Non CVS Files", opt_hideNotInCVS);
    config->writeEntry("Hide Empty Directories", opt_hideEmptyDirectories);
    QValueList<int> sizes = splitter->sizes();
    config->writeEntry("Splitter Pos 1", sizes[0]);
    config->writeEntry("Splitter Pos 2", sizes[1]);

    // write to disk
    config->sync();
}


void CervisiaPart::guiActivateEvent(KParts::GUIActivateEvent* event)
{
    if( event->activated() && cvsService )
    {
        // initial setup of the menu items' state
        updateActions();
        Cervisia::PluginManager::self()->setPart(this);
    }

    // don't call this as it overwrites Konqueror's caption (if you have a
    // Konqueror with more than one view and switch back to Cervisia)
    //
    // KParts::ReadOnlyPart::guiActivateEvent(event);
}


CervisiaBrowserExtension::CervisiaBrowserExtension( CervisiaPart *p )
    : KParts::BrowserExtension( p, "CervisiaBrowserExtension" )
{
    KGlobal::locale()->insertCatalogue("cervisia");
}

CervisiaBrowserExtension::~CervisiaBrowserExtension()
{

}


void CervisiaBrowserExtension::setPropertiesActionEnabled(bool enabled)
{
    emit enableAction("properties", enabled);
}


void CervisiaBrowserExtension::properties()
{
    static_cast<CervisiaPart*>(parent())->slotFileProperties();
}

// Local Variables:
// c-basic-offset: 4
// End:
