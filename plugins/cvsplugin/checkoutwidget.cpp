/*
 * Copyright (c) 2005-2006 Christian Loose <christian.loose@kdemail.net>
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

#include "cvspluginsettings.h"
#include "fetchbranchtagscommand.h"
#include "fetchmodulelistcommand.h"

#include <kdebug.h>


CheckoutWidget::CheckoutWidget(QWidget* parent)
    : CheckoutWidgetBase(parent)
    , m_cmd(0)
    , m_moduleCmd(0)
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

    connect( m_repositoryCombo, SIGNAL(textChanged(const QString&)),
             this, SLOT(repositoryTextChanged()) );

    //
    // module
    //
    m_moduleCombo = new QComboBox(true, this);

    m_fetchModulesBtn = new QPushButton(i18n("Fetch &List"), this);
    connect( m_fetchModulesBtn, SIGNAL(clicked()),
             this, SLOT(moduleButtonClicked()) );

    QBoxLayout* moduleLayout = new QHBoxLayout();
    grid->addLayout(moduleLayout, 1, 1);
    moduleLayout->addWidget(m_moduleCombo, 10);
    moduleLayout->addWidget(m_fetchModulesBtn, 0, AlignVCenter);

    QLabel* moduleLbl = new QLabel(m_moduleCombo, i18n("&Module:"), this);
    grid->addWidget(moduleLbl, 1, 0, AlignLeft | AlignVCenter);

    //
    // branch
    //
    m_branchCombo = new QComboBox(true, this);

    m_fetchBranchesBtn = new QPushButton(i18n("Fetch &List"), this);
    connect( m_fetchBranchesBtn, SIGNAL(clicked()),
             this, SLOT(branchButtonClicked()) );

    QBoxLayout* branchLayout = new QHBoxLayout();
    grid->addLayout(branchLayout, 2, 1);
    branchLayout->addWidget(m_branchCombo, 10);
    branchLayout->addWidget(m_fetchBranchesBtn, 0, AlignVCenter);

    QLabel* branchLbl = new QLabel(m_branchCombo, i18n("&Branch tag:"), this);
    grid->addWidget(branchLbl, 2, 0, AlignLeft | AlignVCenter);

    connect( m_branchCombo, SIGNAL(textChanged(const QString&)),
             this, SLOT(branchTextChanged()) );

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
    // checkout as
    //
    m_aliasEdt = new KLineEdit(this);
    grid->addWidget(m_aliasEdt, 4, 1);

    QLabel* aliasLbl = new QLabel(m_aliasEdt, i18n("Chec&k out as:"), this);
    grid->addWidget(aliasLbl, 4, 0, AlignLeft | AlignVCenter);

    //
    // export only
    //
    m_exportChkBox = new QCheckBox(i18n("Ex&port only"), this);
    grid->addMultiCellWidget(m_exportChkBox, 5, 5, 0, 1);

    //
    // recursive checkout
    //
    m_recursiveChkBox = new QCheckBox(i18n("Re&cursive checkout"), this);
    grid->addMultiCellWidget(m_recursiveChkBox, 6, 6, 0, 1);

    addRepositories();
    restoreUserInput();
}


CheckoutWidget::~CheckoutWidget()
{
//     delete m_cmd; m_cmd = 0;
//     delete m_moduleCmd; m_moduleCmd = 0;
}


QString CheckoutWidget::repository() const
{
    return m_repositoryCombo->currentText();
}


QString CheckoutWidget::module() const
{
    return m_moduleCombo->currentText();
}


QString CheckoutWidget::branch() const
{
    return m_branchCombo->currentText();
}


bool CheckoutWidget::isRecursive() const
{
    return m_recursiveChkBox->isChecked();
}


QString CheckoutWidget::workingFolder() const
{
    return m_workFolderEdt->text();
}


QString CheckoutWidget::alias() const
{
    return m_aliasEdt->text();
}


bool CheckoutWidget::isExportOnly() const
{
    if( m_exportChkBox->isEnabled() )
        return m_exportChkBox->isChecked();

    return false;
}


bool CheckoutWidget::checkUserInput()
{
    if( repository().isEmpty() )
    {
        KMessageBox::information(this, i18n("Please specify a repository."));
        m_repositoryCombo->setFocus();
        return false;
    }

    if( module().isEmpty() )
    {
        KMessageBox::information(this, i18n("Please specify a module name."));
        m_moduleCombo->setFocus();
        return false;
    }

    QFileInfo fi(workingFolder());
    if( !fi.exists() || !fi.isDir() )
    {
        KMessageBox::information(this,
                                 i18n("Please choose an existing working folder."));
        m_workFolderEdt->setFocus();
        return false;
    }

    if( isExportOnly() && branch().isEmpty() )
    {
        KMessageBox::information(this,
                                 i18n("A branch must be specified for export."));
        m_branchCombo->setFocus();
        return false;
    }

    saveUserInput();
    return true;
}


void CheckoutWidget::dirButtonClicked()
{
    QString dirName = KFileDialog::getExistingDirectory(m_workFolderEdt->text());
    if( !dirName.isEmpty() )
        m_workFolderEdt->setText(dirName);
}


void CheckoutWidget::moduleButtonClicked()
{
    kdDebug(8050) << k_funcinfo << endl;

    m_moduleCmd = new FetchModuleListCommand(repository());
    connect( m_moduleCmd, SIGNAL(jobExited(bool, int)),
             this, SLOT(moduleJobExited(bool, int)) );
    if( m_moduleCmd->prepare() )
    {
        m_moduleCmd->execute();
    }
}


void CheckoutWidget::branchButtonClicked()
{
    if( module().isEmpty() )
    {
        KMessageBox::information(this, i18n("Please specify a module name."));
        return;
    }

    m_cmd = new FetchBranchTagsCommand(repository(), module());
    connect( m_cmd, SIGNAL(jobExited(bool, int)),
             this, SLOT(jobExited(bool, int)) );
    if( m_cmd->prepare() )
    {
        m_cmd->execute();
    }
}


void CheckoutWidget::jobExited(bool /*normalExit*/, int /*status*/)
{
    QStringList branchTagList = m_cmd->branchTagList();
    branchTagList.sort();

    m_branchCombo->clear();
    m_branchCombo->insertStringList(branchTagList);

//     delete m_cmd; m_cmd = 0;
}


void CheckoutWidget::moduleJobExited(bool /*normalExit*/, int /*status*/)
{
    m_moduleCombo->clear();
    m_moduleCombo->insertStringList(m_moduleCmd->moduleList());

//     delete m_moduleCmd; m_moduleCmd = 0;
}


void CheckoutWidget::repositoryTextChanged()
{
    bool enabled = !repository().isEmpty();
    m_fetchModulesBtn->setEnabled(enabled);
    m_fetchBranchesBtn->setEnabled(enabled);
}


void CheckoutWidget::branchTextChanged()
{
    if( branch().isEmpty() )
    {
        m_exportChkBox->setEnabled(false);
        m_exportChkBox->setChecked(false);
    }
    else
    {
        m_exportChkBox->setEnabled(true);
    }
}


void CheckoutWidget::addRepositories()
{
    QStringList cvspassList = Repositories::readCvsPassFile();

    QStringList::ConstIterator it;
    for( it = cvspassList.begin(); it != cvspassList.end(); ++it )
        m_repositoryCombo->insertItem(*it);

    QStringList configList = Repositories::readConfigFile();

    for( it = configList.begin(); it != configList.end(); ++it )
    {
        if( !cvspassList.contains(*it) )
            m_repositoryCombo->insertItem(*it);
    }
}


void CheckoutWidget::saveUserInput()
{
    CvsPluginSettings::setRepository(repository());
    CvsPluginSettings::setModule(module());
    CvsPluginSettings::setBranch(branch());
    CvsPluginSettings::setWorkingFolder(workingFolder());
    CvsPluginSettings::setAlias(alias());
    CvsPluginSettings::setExportOnly(isExportOnly());

    CvsPluginSettings::writeConfig();
}


void CheckoutWidget::restoreUserInput()
{
    m_repositoryCombo->setEditText(CvsPluginSettings::repository());
    m_moduleCombo->setEditText(CvsPluginSettings::module());
    m_branchCombo->setCurrentText(CvsPluginSettings::branch());
    m_workFolderEdt->setText(CvsPluginSettings::workingFolder());
    m_aliasEdt->setText(CvsPluginSettings::alias());
    m_exportChkBox->setChecked(CvsPluginSettings::exportOnly());
    m_recursiveChkBox->setChecked(true);
}

#include "checkoutwidget.moc"