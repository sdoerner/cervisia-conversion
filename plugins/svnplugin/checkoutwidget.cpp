/*
 * Copyright (c) 2006 Christian Loose <christian.loose@kdemail.net>
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

#include "checkoutwidget.h"
using Cervisia::CheckoutWidget;

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qdir.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>

#include <kdialog.h>
#include <kfiledialog.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kurlcompletion.h>

#include <repositories.h>

#include "svnpluginsettings.h"

#include <kdebug.h>


CheckoutWidget::CheckoutWidget(QWidget* parent)
    : CheckoutWidgetBase(parent)
{
    QBoxLayout* mainLayout = new QVBoxLayout(this, KDialog::marginHint(), KDialog::spacingHint());

    QGridLayout* grid = new QGridLayout(mainLayout);

    grid->setColStretch(0, 1);
    grid->setColStretch(1, 20);
    for( int i = 0; i <  4; ++i )
        grid->setRowStretch(i, 0);

    //
    // repository
    //
    m_repositoryCombo = new QComboBox(true, this);
    m_repositoryCombo->setFocus();
    // make sure combobox is smaller than the screen
    m_repositoryCombo->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    grid->addWidget(m_repositoryCombo, 0, 1);

    QLabel* repositoryLbl = new QLabel(m_repositoryCombo, i18n("&Repository:"), this);
    grid->addWidget(repositoryLbl, 0, 0, AlignLeft | AlignVCenter);

    //
    // revision
    //
    m_revisionEdt = new KLineEdit(this);
    grid->addWidget(m_revisionEdt, 1, 1);

    QLabel* revisionLbl = new QLabel(m_revisionEdt, i18n("Re&vision:"), this);
    grid->addWidget(revisionLbl, 1, 0, AlignLeft | AlignVCenter);

    //
    // working folder
    //
    m_workFolderEdt = new KLineEdit(this);
    m_workFolderEdt->setText(QDir::homeDirPath());
    m_workFolderEdt->setMinimumWidth(fontMetrics().width('X') * 40);

    KURLCompletion* comp = new KURLCompletion();
    m_workFolderEdt->setCompletionObject(comp);
    m_workFolderEdt->setAutoDeleteCompletionObject(true);
    connect( m_workFolderEdt, SIGNAL(returnPressed(const QString&)),
             comp, SLOT(addItem(const QString&)) );

    QPushButton* folderSelectBtn = new QPushButton("...", this);
    connect( folderSelectBtn, SIGNAL(clicked()),
             this, SLOT(dirButtonClicked()) );
    folderSelectBtn->setFixedWidth(30);

    QBoxLayout* workFolderLayout = new QHBoxLayout();
    grid->addLayout(workFolderLayout, 3, 1);
    workFolderLayout->addWidget(m_workFolderEdt, 10);
    workFolderLayout->addWidget(folderSelectBtn, 0, AlignVCenter);

    QLabel* workFolderLbl = new QLabel(m_workFolderEdt, i18n("Working &folder:"), this);
    grid->addWidget(workFolderLbl, 3, 0, AlignLeft | AlignVCenter);

    //
    // recursive checkout
    //
    m_recursiveChkBox = new QCheckBox(i18n("Re&cursive checkout"), this);
    grid->addMultiCellWidget(m_recursiveChkBox, 6, 6, 0, 1);

//     addRepositories();
    restoreUserInput();
}


CheckoutWidget::~CheckoutWidget()
{
}


QString CheckoutWidget::repository() const
{
    return m_repositoryCombo->currentText();
}


QString CheckoutWidget::revision() const
{
    return m_revisionEdt->text();
}


bool CheckoutWidget::isRecursive() const
{
    return m_recursiveChkBox->isChecked();
}


QString CheckoutWidget::workingFolder() const
{
    return m_workFolderEdt->text();
}


bool CheckoutWidget::checkUserInput()
{
    if( repository().isEmpty() )
    {
        KMessageBox::information(this, i18n("Please specify a repository."));
        m_repositoryCombo->setFocus();
        return false;
    }

//     QFileInfo fi(workingFolder());
//     if( !fi.exists() || !fi.isDir() )
//     {
//         KMessageBox::information(this,
//                                  i18n("Please choose an existing working folder."));
//         m_workFolderEdt->setFocus();
//         return false;
//     }

    saveUserInput();
    return true;
}


void CheckoutWidget::dirButtonClicked()
{
    QString dirName = KFileDialog::getExistingDirectory(m_workFolderEdt->text());
    if( !dirName.isEmpty() )
        m_workFolderEdt->setText(dirName);
}


// void CheckoutWidget::addRepositories()
// {
//     QStringList cvspassList = Repositories::readCvsPassFile();
// 
//     QStringList::ConstIterator it;
//     for( it = cvspassList.begin(); it != cvspassList.end(); ++it )
//         m_repositoryCombo->insertItem(*it);
// 
//     QStringList configList = Repositories::readConfigFile();
// 
//     for( it = configList.begin(); it != configList.end(); ++it )
//     {
//         if( !cvspassList.contains(*it) )
//             m_repositoryCombo->insertItem(*it);
//     }
// }


void CheckoutWidget::saveUserInput()
{
    SvnPluginSettings::setRepository(repository());
    SvnPluginSettings::setRevision(revision());
    SvnPluginSettings::setWorkingFolder(workingFolder());

    SvnPluginSettings::writeConfig();
}


void CheckoutWidget::restoreUserInput()
{
    m_repositoryCombo->setEditText(SvnPluginSettings::repository());
    m_revisionEdt->setText(SvnPluginSettings::revision());
    m_workFolderEdt->setText(SvnPluginSettings::workingFolder());

    m_recursiveChkBox->setChecked(true);
}

#include "checkoutwidget.moc"
