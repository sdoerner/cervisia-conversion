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


#include "repositorydlg.h"

#include <tqlayout.h>
#include <tqpushbutton.h>
#include <kbuttonbox.h>
#include <kconfig.h>
#include <klistview.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kdebug.h>

#include "addrepositorydlg.h"
#include "cvsservice_stub.h"
#include "misc.h"
#include "progressdlg.h"
#include "repositories.h"


class RepositoryListItem : public KListViewItem
{
public:
    RepositoryListItem(KListView* parent, const TQString& repo, bool loggedin);
    
    void setRsh(const TQString& rsh);
    void setServer(const TQString& server) { m_server = server; }
    void setCompression(int compression);
    void setIsLoggedIn(bool isLoggedIn);
    void setRetrieveCvsignore(bool retrieve) { m_retrieveCvsignore = retrieve; }
    
    TQString repository() const
    {
        return text(0);
    }
    TQString rsh() const
    {
        TQString str = text(1);
        return (str.startsWith("ext (") ? str.mid(5, str.length()-6) 
                                        : TQString());
    }
    TQString server() const { return m_server; }
    int compression() const
    {
        bool ok; 
        int n = text(2).toInt(&ok); 
        return ok ? n : -1;
    }
    bool isLoggedIn() const { return m_isLoggedIn; }
    bool retrieveCvsignore() const { return m_retrieveCvsignore; }

private:
    void changeLoginStatusColumn();
    
private:
    TQString m_server;
    bool    m_isLoggedIn;
    bool    m_retrieveCvsignore;
};


static bool LoginNeeded(const TQString& repository)
{
    return repository.startsWith(":pserver:") ||
           repository.startsWith(":sspi:");
}


RepositoryListItem::RepositoryListItem(KListView* parent, const TQString& repo, 
                                       bool loggedin)
    : KListViewItem(parent)
    , m_isLoggedIn(loggedin)
{
    setText(0, repo);

    changeLoginStatusColumn();
}


void RepositoryListItem::setRsh(const TQString& rsh)
{
    TQString repo = repository();
    TQString method;

    if( repo.startsWith(":pserver:") )
        method = "pserver";
    else if( repo.startsWith(":sspi:") )
        method = "sspi";
    else if( repo.contains(':') )
    {
        method = "ext";
        if( !rsh.isEmpty() )
        {
            method += " (";
            method += rsh;
            method += ")";
        }
    }
    else
        method = "local";

    setText(1, method);
}


void RepositoryListItem::setCompression(int compression)
{
    TQString compressionStr = (compression >= 0) ? TQString::number(compression)
                                                : i18n("Default");

    setText(2, compressionStr);
}


void RepositoryListItem::setIsLoggedIn(bool isLoggedIn)
{
    m_isLoggedIn = isLoggedIn;
    
    changeLoginStatusColumn();
}


void RepositoryListItem::changeLoginStatusColumn()
{
    TQString logintqStatus;
    
    if( LoginNeeded(repository()) )
        logintqStatus = m_isLoggedIn ? i18n("Logged in") : i18n("Not logged in");
    else
        logintqStatus = i18n("No login required");
        
    setText(3, logintqStatus);
}


RepositoryDialog::RepositoryDialog(KConfig& cfg, CvsService_stub* cvsService,
                                   TQWidget* parent, const char* name)
    : KDialogBase(parent, name, true, i18n("Configure Access to Repositories"),
                  Ok | Cancel | Help, Ok, true)
    , m_partConfig(cfg)
    , m_cvsService(cvsService)
{
    TQFrame* mainWidget = makeMainWidget();

    TQBoxLayout* hbox = new TQHBoxLayout(mainWidget, 0, spacingHint());

    m_repoList = new KListView(mainWidget);
    hbox->addWidget(m_repoList, 10);
    m_repoList->setMinimumWidth(fontMetrics().width('0') * 60);
    m_repoList->setAllColumnsShowFocus(true);
    m_repoList->addColumn(i18n("Repository"));
    m_repoList->addColumn(i18n("Method"));
    m_repoList->addColumn(i18n("Compression"));
    m_repoList->addColumn(i18n("Status"));
    m_repoList->setFocus();

    connect(m_repoList, TQT_SIGNAL(doubleClicked(TQListViewItem*)),
            this, TQT_SLOT(slotDoubleClicked(TQListViewItem*)));
    connect(m_repoList, TQT_SIGNAL(selectionChanged()),
            this,       TQT_SLOT(slotSelectionChanged()));

    KButtonBox* actionbox = new KButtonBox(mainWidget, Qt::Vertical);
    TQPushButton* addbutton = actionbox->addButton(i18n("&Add..."));
    m_modifyButton = actionbox->addButton(i18n("&Modify..."));
    m_removeButton = actionbox->addButton(i18n("&Remove"));
    actionbox->addStretch();
    m_loginButton  = actionbox->addButton(i18n("Login..."));
    m_logoutButton = actionbox->addButton(i18n("Logout"));
    actionbox->addStretch();
    actionbox->tqlayout();
    hbox->addWidget(actionbox, 0);

    m_loginButton->setEnabled(false);
    m_logoutButton->setEnabled(false);

    connect( addbutton, TQT_SIGNAL(clicked()),
             this, TQT_SLOT(slotAddClicked()) );
    connect( m_modifyButton, TQT_SIGNAL(clicked()),
             this, TQT_SLOT(slotModifyClicked()) );
    connect( m_removeButton, TQT_SIGNAL(clicked()),
             this, TQT_SLOT(slotRemoveClicked()) );
    connect( m_loginButton, TQT_SIGNAL(clicked()),
             this, TQT_SLOT(slotLoginClicked()) );
    connect( m_logoutButton, TQT_SIGNAL(clicked()),
             this, TQT_SLOT(slotLogoutClicked()) );

    // open cvs DCOP service configuration file
    m_serviceConfig = new KConfig("cvsservicerc");

    readCvsPassFile();
    readConfigFile();

    if (TQListViewItem* item = m_repoList->firstChild())
    {
        m_repoList->setCurrentItem(item);
        m_repoList->setSelected(item, true);
    }
    else
    {
        // we have no item so disable modify and remove button
        slotSelectionChanged();
    }

    setHelp("accessing-repository");

    setWFlags(TQt::WDestructiveClose | getWFlags());

    TQSize size = configDialogSize(m_partConfig, "RepositoryDialog");
    resize(size);

    // without this restoreLayout() can't change the column widths
    for (int i = 0; i < m_repoList->columns(); ++i)
        m_repoList->setColumnWidthMode(i, TQListView::Manual);

    m_repoList->restoreLayout(&m_partConfig, TQString::tqfromLatin1("RepositoryListView"));
}


RepositoryDialog::~RepositoryDialog()
{
    saveDialogSize(m_partConfig, "RepositoryDialog");

    m_repoList->saveLayout(&m_partConfig, TQString::tqfromLatin1("RepositoryListView"));

    delete m_serviceConfig;
}


void RepositoryDialog::readCvsPassFile()
{
    TQStringList list = Repositories::readCvsPassFile();
    TQStringList::ConstIterator it;
    for( it = list.begin(); it != list.end(); ++it )
        (void) new RepositoryListItem(m_repoList, (*it), true);
}


void RepositoryDialog::readConfigFile()
{
    TQStringList list = Repositories::readConfigFile();

    // Sort out all list elements which are already in the list view
    TQListViewItem* item = m_repoList->firstChild();
    for( ; item; item = item->nextSibling() )
        list.remove(item->text(0));

    TQStringList::ConstIterator it;
    for( it = list.begin(); it != list.end(); ++it )
        new RepositoryListItem(m_repoList, *it, false);

    // Now look for the used methods
    item = m_repoList->firstChild();
    for( ; item; item = item->nextSibling() )
    {
        RepositoryListItem* ritem = static_cast<RepositoryListItem*>(item);

        // read entries from cvs DCOP service configuration
        m_serviceConfig->setGroup(TQString::tqfromLatin1("Repository-") +
                                  ritem->repository());

        TQString rsh       = m_serviceConfig->readEntry("rsh", TQString());
        TQString server    = m_serviceConfig->readEntry("cvs_server", TQString());
        int compression   = m_serviceConfig->readNumEntry("Compression", -1);
        bool retrieveFile = m_serviceConfig->readBoolEntry("RetrieveCvsignore",
                                                           false);

        ritem->setRsh(rsh);
        ritem->setServer(server);
        ritem->setCompression(compression);
        ritem->setRetrieveCvsignore(retrieveFile);
    }
}


void RepositoryDialog::slotOk()
{
    // Make list of repositories
    TQListViewItem* item;
    TQStringList list;
    for( item = m_repoList->firstChild(); item; item = item->nextSibling() )
        list.append(item->text(0));

    m_partConfig.setGroup("Repositories");
    m_partConfig.writeEntry("Repos", list);

    for( item = m_repoList->firstChild(); item; item = item->nextSibling() )
    {
        RepositoryListItem* ritem = static_cast<RepositoryListItem*>(item);
        
        // write entries to cvs DCOP service configuration
        writeRepositoryData(ritem);
    }

    // write to disk so other services can reparse the configuration
    m_serviceConfig->sync();

    KDialogBase::slotOk();
}


void RepositoryDialog::slotAddClicked()
{
    AddRepositoryDialog dlg(m_partConfig, TQString(), this);
    // default compression level
    dlg.setCompression(-1);
    if( dlg.exec() )
    {
        TQString repo      = Cervisia::NormalizeRepository(dlg.repository());
        TQString rsh       = dlg.rsh();
        TQString server    = dlg.server();
        int compression   = dlg.compression();
        bool retrieveFile = dlg.retrieveCvsignoreFile();

        TQListViewItem* item = m_repoList->firstChild();
        for( ; item; item = item->nextSibling() )
            if( item->text(0) == repo )
            {
                KMessageBox::information(this, i18n("This repository is already known."));
                return;
            }

        RepositoryListItem* ritem = new RepositoryListItem(m_repoList, repo, false);
        ritem->setRsh(rsh);
        ritem->setCompression(compression);
        ritem->setRetrieveCvsignore(retrieveFile);

        // write entries to cvs DCOP service configuration
        writeRepositoryData(ritem);

        // write to disk so other services can reparse the configuration
        m_serviceConfig->sync();
    }
}


void RepositoryDialog::slotModifyClicked()
{
    slotDoubleClicked(m_repoList->selectedItem());
}


void RepositoryDialog::slotRemoveClicked()
{
    // logout from pserver accounts so that they don't
    // get re-added because of the .cvspass file. (BR #51129)
    if( m_logoutButton->isEnabled() )
        slotLogoutClicked();

    delete m_repoList->currentItem();
}


void RepositoryDialog::slotDoubleClicked(TQListViewItem* item)
{
    if( !item )
        return;

    RepositoryListItem* ritem = static_cast<RepositoryListItem*>(item);
    TQString repo      = ritem->repository();
    TQString rsh       = ritem->rsh();
    TQString server    = ritem->server();
    int compression   = ritem->compression();
    bool retrieveFile = ritem->retrieveCvsignore();

    AddRepositoryDialog dlg(m_partConfig, repo, this);
    dlg.setRepository(repo);
    dlg.setRsh(rsh);
    dlg.setServer(server);
    dlg.setCompression(compression);
    dlg.setRetrieveCvsignoreFile(retrieveFile);
    if( dlg.exec() )
    {
        ritem->setRsh(dlg.rsh());
        ritem->setServer(dlg.server());
        ritem->setCompression(dlg.compression());
        ritem->setRetrieveCvsignore(dlg.retrieveCvsignoreFile());

        // write entries to cvs DCOP service configuration
        writeRepositoryData(ritem);

        // write to disk so other services can reparse the configuration
        m_serviceConfig->sync();
    }
}


void RepositoryDialog::slotLoginClicked()
{
    RepositoryListItem* item = (RepositoryListItem*)m_repoList->currentItem();
    if( !item )
        return;

    kdDebug(8050) << k_funcinfo << "repository = " << item->repository() << endl;

    DCOPRef job = m_cvsService->login(item->repository());
    if( !m_cvsService->ok() )
    {
        kdError(8050) << "Failed to call login() method of the cvs DCOP service "
                      << "(" << m_cvsService->app() << ")" << endl;
        return;
    }

    bool success = job.call("execute()");
    if( !success )
    {
        TQStringList output = job.call("output()");
        KMessageBox::detailedError(this, i18n("Login failed."), output.join("\n"));
        return;
    }

    item->setIsLoggedIn(true);
    slotSelectionChanged();
}


void RepositoryDialog::slotLogoutClicked()
{
    RepositoryListItem* item = (RepositoryListItem*)m_repoList->currentItem();
    if( !item )
        return;

    kdDebug(8050) << k_funcinfo << "repository = " << item->repository() << endl;

    DCOPRef job = m_cvsService->logout(item->repository());
    if( !m_cvsService->ok() )
    {
        kdError(8050) << "Failed to call logout() method of the cvs DCOP service "
                      << "(" << m_cvsService->app() << ")" << endl;
        return;
    }

    ProgressDialog dlg(this, "Logout", job, "logout", i18n("CVS Logout"));
    if( !dlg.execute() )
        return;

    item->setIsLoggedIn(false);
    slotSelectionChanged();
}


void RepositoryDialog::slotSelectionChanged()
{
    // retrieve the selected item
    RepositoryListItem* item = (RepositoryListItem*)m_repoList->selectedItem();

    // is an item in the list view selected?
    bool isItemSelected = (item != 0);
    m_modifyButton->setEnabled(isItemSelected);
    m_removeButton->setEnabled(isItemSelected);
    m_loginButton->setEnabled(isItemSelected);
    m_logoutButton->setEnabled(isItemSelected);

    if( !isItemSelected )
        return;

    // is this a pserver repository?
    if( !LoginNeeded(item->repository()) )
    {
        m_loginButton->setEnabled(false);
        m_logoutButton->setEnabled(false);
        return;
    }

    // are we logged in?
    bool isLoggedIn = item->isLoggedIn();
    m_loginButton->setEnabled(!isLoggedIn);
    m_logoutButton->setEnabled(isLoggedIn);
}


void RepositoryDialog::writeRepositoryData(RepositoryListItem* item)
{
    // write entries to cvs DCOP service configuration
    m_serviceConfig->setGroup(TQString::tqfromLatin1("Repository-") +
                              item->repository());

    m_serviceConfig->writeEntry("rsh", item->rsh());
    m_serviceConfig->writeEntry("cvs_server", item->server());
    m_serviceConfig->writeEntry("Compression", item->compression());
    m_serviceConfig->writeEntry("RetrieveCvsignore", item->retrieveCvsignore());
}

#include "repositorydlg.moc"

// kate: space-indent on; indent-width 4; replace-tabs on;
