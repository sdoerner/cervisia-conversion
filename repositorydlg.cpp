/*
 *  Copyright (C) 1999-2002 Bernd Gehrmann
 *                          bernd@mail.berlios.de
 *
 * This program may be distributed under the terms of the Q Public
 * License as defined by Trolltech AS of Norway and appearing in the
 * file LICENSE.QPL included in the packaging of this file.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */


#include "repositorydlg.h"

#include <qlayout.h>
#include <qpushbutton.h>
#include <kbuttonbox.h>
#include <kconfig.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "addrepositorydlg.h"
#include "repositories.h"
#include "listview.h"
#include "cervisiapart.h"


class RepositoryListItem : public QListViewItem
{
public:
    RepositoryListItem(QListView *parent, const QString &repo, bool loggedin);
    void setRsh(const QString &rsh);
    void setCompression(int compression);
    QString repository() const
    {
        return text(0);
    }
    QString rsh() const
    {
        QString str = text(1);
        return (str.startsWith("ext (")? str.mid(5, str.length()-6) : QString::null);
    }
    int compression() const
    {
        bool ok; int n = text(2).toInt(&ok); return ok? n : -1;
    }
};


RepositoryListItem::RepositoryListItem(QListView *parent, const QString &repo, bool loggedin)
    : QListViewItem(parent)
{
    setText(0, repo);

    QString status;
    if (repo.startsWith(":pserver:"))
        status = loggedin? i18n("Logged in") : i18n("Not logged in");
    else
        status = i18n("No login required");
    setText(3, status);
}


void RepositoryListItem::setRsh(const QString &rsh)
{
    QString repo = repository();
    QString method;
    
    if (repo.startsWith(":pserver:"))
        method = "pserver";
    else if (repo.contains(':'))
        {
            method = "ext";
            if (!rsh.isEmpty())
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
    QString compressionStr = (compression >= 0)?
        QString::number(compression) : i18n("Default");
    
    setText(2, compressionStr);
}


RepositoryDialog::Options *RepositoryDialog::options = 0;


RepositoryDialog::RepositoryDialog(QWidget *parent, const char *name)
    : KDialogBase(parent, name, true, i18n("Configure Access to Repositories"),
                  Ok | Cancel | Help, Ok, true)
{
    QFrame* mainWidget = makeMainWidget();

    QBoxLayout *layout = new QVBoxLayout(mainWidget, 0, spacingHint());

    QBoxLayout *hbox = new QHBoxLayout(layout);

    repolist = new ListView(mainWidget);
    hbox->addWidget(repolist, 10);
    repolist->setMinimumWidth(fontMetrics().width('0') * 60);
    repolist->setAllColumnsShowFocus(true);
    repolist->setPreferredColumn(0);
    repolist->addColumn(i18n("Repository"));
    repolist->addColumn(i18n("Method"));
    repolist->addColumn(i18n("Compression"));
    repolist->addColumn(i18n("Status"));
    repolist->setFocus();

    KButtonBox *actionbox = new KButtonBox(mainWidget, KButtonBox::Vertical);
    actionbox->addStretch();
    QPushButton *addbutton = actionbox->addButton(i18n("&Add..."));
    QPushButton *removebutton = actionbox->addButton(i18n("&Remove"));
    QPushButton *settingsbutton = actionbox->addButton(i18n("&Settings..."));
#if 0
    actionbox->addStretch();
    QPushButton *loginbutton = actionbox->addButton(i18n("Login..."));
    QPushButton *logoutbutton = actionbox->addButton(i18n("Logout..."));
#endif
    actionbox->addStretch();
    actionbox->layout();
    hbox->addWidget(actionbox, 0);

    connect( addbutton, SIGNAL(clicked()),
             this, SLOT(slotAddClicked()) );
    connect( removebutton, SIGNAL(clicked()),
             this, SLOT(slotRemoveClicked()) );
    connect( settingsbutton, SIGNAL(clicked()),
             this, SLOT(slotSettingsClicked()) );
#if 0
    connect( loginbutton, SIGNAL(clicked()),
             this, SLOT(slotLoginClicked()) );
    connect( logoutbutton, SIGNAL(clicked()),
             this, SLOT(slotLogoutClicked()) );
#endif

    // open cvs DCOP service configuration file
    serviceConfig = new KConfig("cvsservicerc");
    
    readCvsPassFile();
    readConfigFile();

    setHelp("accessing-repository");

    if (options)
        resize(options->size);
}


RepositoryDialog::~RepositoryDialog()
{
    if (!options)
        options = new Options;
    options->size = size();
    
    delete serviceConfig;
}


void RepositoryDialog::loadOptions(KConfig *config)
{
    if (!config->readEntry("Customized"))
        return;

    options = new Options;
    options->size = config->readSizeEntry("Size");

    config->setGroup("AddRepository dialog");
    AddRepositoryDialog::loadOptions(config);
}


void RepositoryDialog::saveOptions(KConfig *config)
{
    if (!options)
        return;
    
    config->writeEntry("Customized", true);
    config->writeEntry("Size", options->size);

    config->setGroup("AddRepository dialog");
    AddRepositoryDialog::saveOptions(config);
}


void RepositoryDialog::readCvsPassFile()
{
    QStringList list = Repositories::readCvsPassFile();
    QStringList::ConstIterator it;
    for (it = list.begin(); it != list.end(); ++it)
        (void) new RepositoryListItem(repolist, (*it), true);
}


void RepositoryDialog::readConfigFile()
{
    QStringList list = Repositories::readConfigFile();
    
    // Sort out all list elements which are already in the list view
    QListViewItem *item = repolist->firstChild();
    for ( ; item; item = item->nextSibling())
        list.remove(item->text(0));

    QStringList::ConstIterator it;
    for (it = list.begin(); it != list.end(); ++it)
        new RepositoryListItem(repolist, *it, false);
            
    // Now look for the used methods
    item = repolist->firstChild();
    for (; item; item = item->nextSibling())
        {
            RepositoryListItem *ritem = static_cast<RepositoryListItem*>(item);

            // read entries from cvs DCOP service configuration
            serviceConfig->setGroup(QString::fromLatin1("Repository-") + ritem->repository());
            QString rsh = serviceConfig->readEntry("rsh", QString());
            int compression = serviceConfig->readNumEntry("Compression", -1);

            ritem->setRsh(rsh);
            ritem->setCompression(compression);
        }
}


void RepositoryDialog::slotOk()
{
    // Make list of repositories
    QListViewItem *item;
    QStringList list;
    for (item = repolist->firstChild(); item; item = item->nextSibling())
        list.append(item->text(0));

    KConfig *config = CervisiaPart::config();
    config->setGroup("Repositories");
    config->writeEntry("Repos", list);

    for (item = repolist->firstChild(); item; item = item->nextSibling())
    {
        RepositoryListItem *ritem = static_cast<RepositoryListItem*>(item);
        // write entries to cvs DCOP service configuration
        serviceConfig->setGroup(QString::fromLatin1("Repository-") + ritem->repository());
        serviceConfig->writeEntry("rsh", ritem->rsh());
        serviceConfig->writeEntry("Compression", ritem->compression());

        // TODO: remove when move to cvs DCOP service is complete
        config->setGroup(QString("Repository-") + ritem->repository());
        config->writeEntry("rsh", ritem->rsh());
        config->writeEntry("Compression", ritem->compression());
        // END TODO
    }

    // write to disk so other services can reparse the configuration
    serviceConfig->sync();

    KDialogBase::slotOk();
}


void RepositoryDialog::slotAddClicked()
{
    AddRepositoryDialog dlg(QString::null, this);
    // default compression level
    dlg.setCompression(-1);
    if (dlg.exec())
        {
            QString repo = dlg.repository();
            QString rsh = dlg.rsh();
            int compression = dlg.compression();
            
            QListViewItem *item = repolist->firstChild();
            for ( ; item; item = item->nextSibling())
                if (item->text(0) == repo)
                    {
                        KMessageBox::information(this, "Cervisia",
                                                 i18n("This repository is already known."));
                        return;
                    }
            
            RepositoryListItem *ritem = new RepositoryListItem(repolist, repo, false);
            ritem->setRsh(rsh);
            ritem->setCompression(compression);

            // write entries to cvs DCOP service configuration
            serviceConfig->setGroup(QString::fromLatin1("Repository-") + ritem->repository());
            serviceConfig->writeEntry("rsh", ritem->rsh());
            serviceConfig->writeEntry("Compression", ritem->compression());

            // write to disk so other services can reparse the configuration
            serviceConfig->sync();

            // TODO: remove when move to cvs DCOP service is complete
            KConfig *config = CervisiaPart::config();
            config->setGroup(QString("Repository-") + repo);
            config->writeEntry("rsh", rsh);
            config->writeEntry("Compression", compression);
            // END TODO
        }
}


void RepositoryDialog::slotRemoveClicked()
{
    delete repolist->currentItem();
}


void RepositoryDialog::slotDoubleClicked(QListViewItem *item)
{
    if (!item)
        return;

    RepositoryListItem *ritem = static_cast<RepositoryListItem*>(item);
    QString repo = ritem->repository();
    QString rsh = ritem->rsh();
    int compression = ritem->compression();
    
    AddRepositoryDialog dlg(repo, this);
    dlg.setRepository(repo);
    dlg.setRsh(rsh);
    dlg.setCompression(compression);
    if (dlg.exec())
        {
            ritem->setRsh(dlg.rsh());
            ritem->setCompression(dlg.compression());

            // write entries to cvs DCOP service configuration
            serviceConfig->setGroup(QString::fromLatin1("Repository-") + ritem->repository());
            serviceConfig->writeEntry("rsh", ritem->rsh());
            serviceConfig->writeEntry("Compression", ritem->compression());

            // write to disk so other services can reparse the configuration
            serviceConfig->sync();

            // TODO: remove when move to cvs DCOP service is complete
            KConfig *config = CervisiaPart::config();
            config->setGroup(QString("Repository-") + repo);
            config->writeEntry("rsh", dlg.rsh());
            config->writeEntry("Compression", dlg.compression());
            // END TODO
        }
}


void RepositoryDialog::slotSettingsClicked()
{
    slotDoubleClicked(repolist->currentItem());
}


void RepositoryDialog::slotLoginClicked()
{
}


void RepositoryDialog::slotLogoutClicked()
{
}

#include "repositorydlg.moc"


// Local Variables:
// c-basic-offset: 4
// End:
