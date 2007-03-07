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

#include "settingsdlg.h"

#include <qapplication.h>
#include <qcheckbox.h>
#include <q3grid.h>
#include <q3groupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <q3vbox.h>
#include <qwidget.h>
#include <q3buttongroup.h>
#include <qradiobutton.h>
//Added by qt3to4:
#include <QPixmap>
#include <QVBoxLayout>
#include <kbuttonbox.h>
#include <kcolorbutton.h>
#include <kconfig.h>
#include <kfontdialog.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <klocale.h>
#include <knuminput.h>
#include <kurlrequester.h>
#include <kcomponentdata.h>
#include <kvbox.h>
#include "misc.h"
#include "cervisiasettings.h"
#include "settingsdlg_advanced.h"


namespace
{
    // helper method to load icons for configuration pages
    inline QPixmap LoadIcon(const char* iconName)
    {
        return KIconLoader::global()->loadIcon(QLatin1String(iconName), K3Icon::NoGroup,
                                 K3Icon::SizeMedium);
    }
}


FontButton::FontButton( const QString &text, QWidget *parent, const char *name )
    : QPushButton(text, parent, name)
{
    connect( this, SIGNAL(clicked()), this, SLOT(chooseFont()) );
}


void FontButton::chooseFont()
{
    QFont newFont(font());

    if (KFontDialog::getFont(newFont, false, this) == QDialog::Rejected)
        return;

    setFont(newFont);
    repaint();
}


SettingsDialog::SettingsDialog( KConfig *conf, QWidget *parent, const char *name )
    : KPageDialog(parent)
{
    setFaceType( List );
    setCaption(i18n("Configure Cervisia"));
    setButtons(Ok | Cancel | Help);
    setDefaultButton(Ok);
    showButtonSeparator(true);

    config = conf;

    // open cvs D-Bus service configuration file
    serviceConfig = new KConfig("cvsservicerc");

    //
    // General Options
    //
    addGeneralPage();

    //
    // Diff Options
    //
    addDiffPage();

    //
    // Status Options
    //
    addStatusPage();

    //
    // Advanced Options
    //
    addAdvancedPage();

    //
    // Look and Feel Options
    //
    addLookAndFeelPage();

    readSettings();

    setHelp("customization", "cervisia");
}

SettingsDialog::~SettingsDialog()
{
    delete serviceConfig;
}

void SettingsDialog::readSettings()
{
    // read entries from cvs D-Bus service configuration
    serviceConfig->setGroup("General");
    cvspathedit->setUrl(serviceConfig->readPathEntry("CVSPath", "cvs"));
    m_advancedPage->kcfg_Compression->setValue(serviceConfig->readEntry(
                                                   "Compression", 0));
    m_advancedPage->kcfg_UseSshAgent->setChecked(serviceConfig->readEntry(
                                                   "UseSshAgent", false));

    config->setGroup("General");
    m_advancedPage->kcfg_Timeout->setValue(CervisiaSettings::timeout());
    usernameedit->setText(config->readEntry("Username", Cervisia::UserName()));

    contextedit->setValue(config->readEntry("ContextLines", 65535));
    tabwidthedit->setValue(config->readEntry("TabWidth", 8));
    diffoptedit->setText(config->readEntry("DiffOptions"));
    extdiffedit->setUrl(config->readPathEntry("ExternalDiff"));
    remotestatusbox->setChecked(config->readEntry("StatusForRemoteRepos", false));
    localstatusbox->setChecked(config->readEntry("StatusForLocalRepos", false));

    // read configuration for look and feel page
    config->setGroup("LookAndFeel");
    m_protocolFontBox->setFont(config->readEntry("ProtocolFont",QFont()));
    m_annotateFontBox->setFont(config->readEntry("AnnotateFont",QFont()));
    m_diffFontBox->setFont(config->readEntry("DiffFont",QFont()));
    m_changelogFontBox->setFont(config->readEntry("ChangeLogFont",QFont()));
    m_splitterBox->setChecked(config->readEntry("SplitHorizontally",true));

    m_conflictButton->setColor(CervisiaSettings::conflictColor());
    m_localChangeButton->setColor(CervisiaSettings::localChangeColor());
    m_remoteChangeButton->setColor(CervisiaSettings::remoteChangeColor());
    m_notInCvsButton->setColor(CervisiaSettings::notInCvsColor());

    m_diffChangeButton->setColor(CervisiaSettings::diffChangeColor());
    m_diffInsertButton->setColor(CervisiaSettings::diffInsertColor());
    m_diffDeleteButton->setColor(CervisiaSettings::diffDeleteColor());
}


void SettingsDialog::writeSettings()
{
    // write entries to cvs D-Bus service configuration
    serviceConfig->setGroup("General");
    serviceConfig->writePathEntry("CVSPath", cvspathedit->url().path());
    serviceConfig->writeEntry("Compression",
        m_advancedPage->kcfg_Compression->value());
    serviceConfig->writeEntry("UseSshAgent",
        m_advancedPage->kcfg_UseSshAgent->isChecked());

    // write to disk so other services can reparse the configuration
    serviceConfig->sync();

    config->setGroup("General");
    CervisiaSettings::setTimeout(m_advancedPage->kcfg_Timeout->value());
    config->writeEntry("Username", usernameedit->text());

    config->writePathEntry("ExternalDiff", extdiffedit->url().path());

    config->writeEntry("ContextLines", (unsigned)contextedit->value());
    config->writeEntry("TabWidth", tabwidthedit->value());
    config->writeEntry("DiffOptions", diffoptedit->text());
    config->writeEntry("StatusForRemoteRepos", remotestatusbox->isChecked());
    config->writeEntry("StatusForLocalRepos", localstatusbox->isChecked());

    config->setGroup("LookAndFeel");
    config->writeEntry("ProtocolFont", m_protocolFontBox->font());
    config->writeEntry("AnnotateFont", m_annotateFontBox->font());
    config->writeEntry("DiffFont", m_diffFontBox->font());
    config->writeEntry("ChangeLogFont", m_changelogFontBox->font());
    config->writeEntry("SplitHorizontally", m_splitterBox->isChecked());

    CervisiaSettings::setConflictColor(m_conflictButton->color());
    CervisiaSettings::setLocalChangeColor(m_localChangeButton->color());
    CervisiaSettings::setRemoteChangeColor(m_remoteChangeButton->color());
    CervisiaSettings::setNotInCvsColor(m_notInCvsButton->color());
    CervisiaSettings::setDiffChangeColor(m_diffChangeButton->color());
    CervisiaSettings::setDiffInsertColor(m_diffInsertButton->color());
    CervisiaSettings::setDiffDeleteColor(m_diffDeleteButton->color());

    // I'm not yet sure whether this is a hack or not :-)
#ifdef __GNUC__
#warning would QApplication::topLevelWidgets be sufficient?
#endif
    const QWidgetList& widgets = QApplication::allWidgets();
    Q_FOREACH (QWidget* w, widgets)
    {
        if (w->inherits("ProtocolView"))
            w->setFont(m_protocolFontBox->font());
        if (w->inherits("AnnotateView"))
            w->setFont(m_annotateFontBox->font());
        if (w->inherits("DiffView"))
            w->setFont(m_diffFontBox->font());
    }
    config->sync();

    CervisiaSettings::writeConfig();
}

void SettingsDialog::done(int res)
{
    if (res == Accepted)
        writeSettings();
    KDialog::done(res);
}


/*
 * Create a page for the general options
 */
void SettingsDialog::addGeneralPage()
{
    QFrame* generalPage = new QFrame;
    KPageWidgetItem *page = new KPageWidgetItem( generalPage, i18n("General") );
    page->setIcon( KIcon(LoadIcon("misc")) );
    addPage(page);
    
    QVBoxLayout* layout = new QVBoxLayout(generalPage);
    layout->setSpacing(KDialog::spacingHint());
    layout->setMargin(0);

    QLabel *usernamelabel = new QLabel( i18n("&User name for the change log editor:"), generalPage );
    usernameedit = new KLineEdit(generalPage);
    usernameedit->setFocus();
    usernamelabel->setBuddy(usernameedit);

    layout->addWidget(usernamelabel);
    layout->addWidget(usernameedit);

    QLabel *cvspathlabel = new QLabel( i18n("&Path to CVS executable, or 'cvs':"), generalPage );
    cvspathedit = new KUrlRequester(generalPage);
    cvspathlabel->setBuddy(cvspathedit);

    layout->addWidget(cvspathlabel);
    layout->addWidget(cvspathedit);

    layout->addStretch();
}


/*
 * Create a page for the diff optionsw
 */
void SettingsDialog::addDiffPage()
{
    QFrame* diffPage = new QFrame;
    KPageWidgetItem *page = new KPageWidgetItem( diffPage, i18n("Diff Viewer") );
    page->setIcon( KIcon(LoadIcon("vcs_diff")) );
    addPage(page);



    QGridLayout* layout = new QGridLayout(diffPage);

    QLabel *contextlabel = new QLabel( i18n("&Number of context lines in diff dialog:"), diffPage );
    contextedit = new KIntNumInput( 0, diffPage );
    contextedit->setRange(0, 65535, 1, false);
    contextlabel->setBuddy(contextedit);

    layout->addWidget(contextlabel, 0, 0);
    layout->addWidget(contextedit, 0, 1);

    QLabel *diffoptlabel = new QLabel(i18n("Additional &options for cvs diff:"), diffPage);
    diffoptedit = new KLineEdit(diffPage);
    diffoptlabel->setBuddy(diffoptedit);

    layout->addWidget(diffoptlabel, 1, 0);
    layout->addWidget(diffoptedit, 1, 1);

    QLabel *tabwidthlabel = new QLabel(i18n("Tab &width in diff dialog:"), diffPage);
    tabwidthedit = new KIntNumInput(0, diffPage);
    tabwidthedit->setRange(1, 16, 1, false);
    tabwidthlabel->setBuddy(tabwidthedit);

    layout->addWidget(tabwidthlabel, 2, 0);
    layout->addWidget(tabwidthedit, 2, 1);

    QLabel *extdifflabel = new QLabel(i18n("External diff &frontend:"), diffPage);
    extdiffedit = new KUrlRequester(diffPage);
    extdifflabel->setBuddy(extdiffedit);

    layout->addWidget(extdifflabel, 3, 0);
    layout->addWidget(extdiffedit, 3, 1);

    // add dummy row to take up the vertical space
    layout->addWidget(0, 4, 0, 0, 1);
    layout->setRowStretch(4, 10);
}


/*
 * Create a page for the status options
 */
void SettingsDialog::addStatusPage()
{
    KVBox* statusPage = new KVBox;
    KPageWidgetItem *page = new KPageWidgetItem( statusPage, i18n("Status") );
    page->setIcon( KIcon(LoadIcon("fork")) );
    addPage(page);


    remotestatusbox = new QCheckBox(i18n("When opening a sandbox from a &remote repository,\n"
                                         "start a File->Status command automatically"), statusPage);
    localstatusbox = new QCheckBox(i18n("When opening a sandbox from a &local repository,\n"
                                        "start a File->Status command automatically"), statusPage);

    // dummy widget to take up the vertical space
    new QWidget(statusPage);
}


/*
 * Create a page for the advanced options
 */
void SettingsDialog::addAdvancedPage()
{
    KVBox* frame = new KVBox;
    KPageWidgetItem *page = new KPageWidgetItem( frame, i18n("Advanced") );
    page->setIcon( KIcon(LoadIcon("configure")) );
    addPage(page);

    m_advancedPage = new AdvancedPage(frame);
    m_advancedPage->kcfg_Timeout->setRange(0, 50000, 100, false);
    m_advancedPage->kcfg_Compression->setRange(0, 9, 1, false);
}


/*
 * Create a page for the look & feel options
 */
void SettingsDialog::addLookAndFeelPage()
{
    KVBox* lookPage = new KVBox;
    KPageWidgetItem *page = new KPageWidgetItem( lookPage, i18n("Appearance") );
    page->setIcon( KIcon(LoadIcon("looknfeel")) );
    addPage(page);


    Q3GroupBox* fontGroupBox = new Q3GroupBox(4, Qt::Vertical, i18n("Fonts"),
                                            lookPage);
    fontGroupBox->setInsideSpacing(KDialog::spacingHint());

    m_protocolFontBox  = new FontButton(i18n("Font for &Protocol Window..."),
                                        fontGroupBox);
    m_annotateFontBox  = new FontButton(i18n("Font for A&nnotate View..."),
                                        fontGroupBox);
    m_diffFontBox      = new FontButton(i18n("Font for D&iff View..."),
                                        fontGroupBox);
    m_changelogFontBox = new FontButton(i18n("Font for ChangeLog View..."),
                                        fontGroupBox);

    Q3GroupBox* colorGroupBox = new Q3GroupBox(4, Qt::Horizontal,
                                             i18n("Colors"), lookPage);
    colorGroupBox->setColumns(4);
    colorGroupBox->setInsideSpacing(KDialog::spacingHint());

    QLabel* conflictLabel = new QLabel(i18n("Conflict:"), colorGroupBox);
    m_conflictButton      = new KColorButton(colorGroupBox);
    conflictLabel->setBuddy(m_conflictButton);

    QLabel* diffChangeLabel = new QLabel(i18n("Diff change:"), colorGroupBox);
    m_diffChangeButton      = new KColorButton(colorGroupBox);
    diffChangeLabel->setBuddy(m_diffChangeButton);

    QLabel* localChangeLabel = new QLabel(i18n("Local change:"), colorGroupBox);
    m_localChangeButton      = new KColorButton(colorGroupBox);
    localChangeLabel->setBuddy(m_localChangeButton);

    QLabel* diffInsertLabel = new QLabel(i18n("Diff insertion:"), colorGroupBox);
    m_diffInsertButton      = new KColorButton(colorGroupBox);
    diffInsertLabel->setBuddy(m_diffInsertButton);

    QLabel* remoteChangeLabel = new QLabel(i18n("Remote change:"), colorGroupBox);
    m_remoteChangeButton      = new KColorButton(colorGroupBox);
    remoteChangeLabel->setBuddy( m_remoteChangeButton );

    QLabel* diffDeleteLabel = new QLabel(i18n("Diff deletion:"), colorGroupBox);
    m_diffDeleteButton      = new KColorButton(colorGroupBox);
    diffDeleteLabel->setBuddy(m_diffDeleteButton);

    QLabel* notInCvsLabel = new QLabel(i18n("Not in cvs:"), colorGroupBox);
    m_notInCvsButton      = new KColorButton(colorGroupBox);
    notInCvsLabel->setBuddy(m_notInCvsButton);

    m_splitterBox = new QCheckBox(i18n("Split main window &horizontally"), lookPage);
}

#include "settingsdlg.moc"


// Local Variables:
// c-basic-offset: 4
// End:
