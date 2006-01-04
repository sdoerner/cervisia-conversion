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
#include <kurlcompletion.h>

#include <repositories.h>

#include "cvspluginsettings.h"

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
    // module
    //
    m_moduleCombo = new QComboBox(true, this);

    QPushButton* fetchModulesBtn = new QPushButton(i18n("Fetch &List"), this);
    connect( fetchModulesBtn, SIGNAL(clicked()),
             this, SLOT(moduleButtonClicked()) );

    QBoxLayout* moduleLayout = new QHBoxLayout();
    grid->addLayout(moduleLayout, 1, 1);
    moduleLayout->addWidget(m_moduleCombo, 10);
    moduleLayout->addWidget(fetchModulesBtn, 0, AlignVCenter);

    QLabel* moduleLbl = new QLabel(m_moduleCombo, i18n("&Module:"), this);
    grid->addWidget(moduleLbl, 1, 0, AlignLeft | AlignVCenter);

    //
    // branch
    //
    m_branchCombo = new QComboBox(true, this);

    QPushButton* fetchBranchesBtn = new QPushButton(i18n("Fetch &List"), this);
    connect( fetchBranchesBtn, SIGNAL(clicked()),
             this, SLOT(branchButtonClicked()) );

    QBoxLayout* branchLayout = new QHBoxLayout();
    grid->addLayout(branchLayout, 2, 1);
    branchLayout->addWidget(m_branchCombo, 10);
    branchLayout->addWidget(fetchBranchesBtn, 0, AlignVCenter);

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
    kdDebug(8050) << k_funcinfo << " -- NOT YET IMPLEMENTED -- " << endl;

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
    kdDebug(8050) << k_funcinfo << " -- NOT YET IMPLEMENTED -- " << endl;

/*    DCOPRef cvsJob = cvsService->moduleList(repository());
    if( !cvsService->ok() )
    return;

    ProgressDialog dlg(this, "Checkout", cvsJob, "checkout", i18n("CVS Checkout"));
    if( !dlg.execute() )
    return;

    module_combo->clear();
    QString str;
    while (dlg.getLine(str))
    {
    if (str.left(12) == "Unknown host")
    continue;

    int pos = str.find(' ');
    if (pos == -1)
    pos = str.find('\t');
    if (pos == -1)
    pos = str.length();
    QString module( str.left(pos).stripWhiteSpace() );
    if ( !module.isEmpty() )
    module_combo->insertItem(module);
}*/
}


void CheckoutWidget::branchButtonClicked()
{
    kdDebug(8050) << k_funcinfo << " -- NOT YET IMPLEMENTED -- " << endl;

//     QStringList branchTagList;
    //
//     if( repository().isEmpty() )
//     {
//         KMessageBox::information(this, i18n("Please specify a repository."));
//         return;
//     }
    //
//     if( module().isEmpty() )
//     {
//         KMessageBox::information(this, i18n("Please specify a module name."));
//         return;
//     }

//     DCOPRef cvsJob = cvsService->rlog(repository(), module(), 
//                                       false/*recursive*/);
/*    if( !cvsService->ok() )
    return;

    ProgressDialog dlg(this, "Remote Log", cvsJob, QString::null,
    i18n("CVS Remote Log"));
    if( !dlg.execute() )
    return;

    QString line;
    while( dlg.getLine(line) )
    {
    int colonPos;

    if( line.isEmpty() || line[0] != '\t' )
    continue;
    if( (colonPos = line.find(':', 1)) < 0 )
    continue;

    const QString tag  = line.mid(1, colonPos - 1);
    if( !branchTagList.contains(tag) )
    branchTagList.push_back(tag);
}

    branchTagList.sort();

    branchCombo->insertStringList(branchTagList);*/
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
    kdDebug(8050) << k_funcinfo << " -- NOT YET IMPLEMENTED -- " << endl;

    CvsPluginSettings::setModule(module());

    CvsPluginSettings::writeConfig();
}


void CheckoutWidget::restoreUserInput()
{
    kdDebug(8050) << k_funcinfo << " -- NOT YET IMPLEMENTED -- " << endl;
    m_moduleCombo->setEditText(CvsPluginSettings::module());
}

#include "checkoutwidget.moc"
