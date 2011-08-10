/*
 *  Copyright (C) 1999-2002 Bernd Gehrmann
 *                          bernd@mail.berlios.de
 *  Copyright (c) 2002-2004 Christian Loose <christian.loose@kdemail.net>
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


#include "addrepositorydlg.h"

#include <tqcheckbox.h>
#include <tqhbox.h>
#include <tqlabel.h>
#include <tqlayout.h>

#include <kconfig.h>
#include <klineedit.h>
#include <klocale.h>
#include <knuminput.h>


AddRepositoryDialog::AddRepositoryDialog(KConfig& cfg, const TQString& repo, 
                                         TQWidget* tqparent, const char* name)
    : KDialogBase(tqparent, name, true, i18n("Add Repository"),
                  Ok | Cancel, Ok, true)
    , partConfig(cfg)
{
    TQFrame* mainWidget = makeMainWidget();

    TQBoxLayout* tqlayout = new TQVBoxLayout(mainWidget, 0, spacingHint());

    TQLabel* repo_label = new TQLabel(i18n("&Repository:"), mainWidget);
    tqlayout->addWidget(repo_label);
    
    repo_edit = new KLineEdit(mainWidget);
    repo_edit->setFocus();
    repo_label->setBuddy(repo_edit);
    if( !repo.isNull() )
    {
        repo_edit->setText(repo);
        repo_edit->setEnabled(false);
    }
    tqlayout->addWidget(repo_edit);
    
    TQLabel* rsh_label = new TQLabel(i18n("Use remote &shell (only for :ext: repositories):"), mainWidget);
    tqlayout->addWidget(rsh_label);
    
    rsh_edit = new KLineEdit(mainWidget);
    rsh_label->setBuddy(rsh_edit);
    tqlayout->addWidget(rsh_edit);
    
    TQLabel* server_label = new TQLabel(i18n("Invoke this program on the server side:"),
                                      mainWidget);
    tqlayout->addWidget(server_label);
    
    server_edit = new KLineEdit(mainWidget);
    server_label->setBuddy(server_edit);
    tqlayout->addWidget(server_edit);

    TQHBox* compressionBox = new TQHBox(mainWidget);
    m_useDifferentCompression = new TQCheckBox(i18n("Use different &compression level:"), compressionBox);
    
    m_compressionLevel = new KIntNumInput(compressionBox);
    m_compressionLevel->setRange(0, 9, 1, false);
    tqlayout->addWidget(compressionBox);
    
    m_retrieveCvsignoreFile = new TQCheckBox(i18n("Download cvsignore file from "
                                            "server"), mainWidget);
    tqlayout->addWidget(m_retrieveCvsignoreFile);

    connect( repo_edit, TQT_SIGNAL(textChanged(const TQString&)),
             this, TQT_SLOT(repoChanged()) );
    connect( m_useDifferentCompression, TQT_SIGNAL(toggled(bool)),
             this, TQT_SLOT(compressionToggled(bool)) );
    repoChanged();

    TQSize size = configDialogSize(partConfig, "AddRepositoryDialog");
    resize(size);
}


AddRepositoryDialog::~AddRepositoryDialog()
{
    saveDialogSize(partConfig, "AddRepositoryDialog");
}


void AddRepositoryDialog::setRsh(const TQString& rsh)
{
    rsh_edit->setText(rsh);
}


void AddRepositoryDialog::setServer(const TQString& server)
{
    server_edit->setText(server);
}


void AddRepositoryDialog::setCompression(int compression)
{
    if( compression < 0 )
    {
        // TODO: use KConfigXT to retrieve default compression level
        m_compressionLevel->setValue(0);
        m_useDifferentCompression->setChecked(false);
    }
    else
    {
        m_useDifferentCompression->setChecked(true);
        m_compressionLevel->setValue(compression);
    }
    
    compressionToggled(m_useDifferentCompression->isChecked());
}


void AddRepositoryDialog::setRetrieveCvsignoreFile(bool enabled)
{
    m_retrieveCvsignoreFile->setChecked(enabled);
}


TQString AddRepositoryDialog::repository() const
{
    return repo_edit->text();
}


TQString AddRepositoryDialog::rsh() const
{
    return rsh_edit->text();
}


TQString AddRepositoryDialog::server() const
{
    return server_edit->text();
}


int AddRepositoryDialog::compression() const
{
    if( m_useDifferentCompression->isChecked() )
        return m_compressionLevel->value();
    else
        return -1;
}


bool AddRepositoryDialog::retrieveCvsignoreFile() const
{
    return m_retrieveCvsignoreFile->isChecked();
}


void AddRepositoryDialog::setRepository(const TQString& repo)
{
    setCaption(i18n("Repository Settings"));

    repo_edit->setText(repo);
    repo_edit->setEnabled(false);
}


void AddRepositoryDialog::repoChanged()
{
    TQString repo = repository();
    rsh_edit->setEnabled((!repo.startsWith(":pserver:"))
                         && repo.contains(":"));
    m_useDifferentCompression->setEnabled(repo.contains(":"));
    if( !repo.contains(":") )
        m_compressionLevel->setEnabled(false);
    else
        compressionToggled(m_useDifferentCompression->isChecked());
}


void AddRepositoryDialog::compressionToggled(bool checked)
{
    m_compressionLevel->setEnabled(checked);
}

#include "addrepositorydlg.moc"


// Local Variables:
// c-basic-offset: 4
// End:
