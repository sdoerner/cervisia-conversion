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

#include <tqapplication.h>
#include <tqcheckbox.h>
#include <tqgrid.h>
#include <tqgroupbox.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqvbox.h>
#include <tqwidgetlist.h>
#include <tqhbuttongroup.h>
#include <tqradiobutton.h>
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

#include "misc.h"
#include "cervisiasettings.h"
#include "settingsdlg_advanced.h"


namespace
{
    // helper method to load icons for configuration pages
    inline TQPixmap LoadIcon(const char* iconName)
    {
        KIconLoader* loader = KGlobal::instance()->iconLoader();
        return loader->loadIcon(TQString::fromLatin1(iconName), KIcon::NoGroup,
                                 KIcon::SizeMedium);
    }
}


FontButton::FontButton( const TQString &text, TQWidget *parent, const char *name )
    : TQPushButton(text, parent, name)
{
    connect( this, TQT_SIGNAL(clicked()), this, TQT_SLOT(chooseFont()) );
}


void FontButton::chooseFont()
{
    TQFont newFont(font());

    if (KFontDialog::getFont(newFont, false, this) == TQDialog::Rejected)
        return;

    setFont(newFont);
    repaint(false);
}


SettingsDialog::SettingsDialog( KConfig *conf, TQWidget *parent, const char *name )
    : KDialogBase(KDialogBase::IconList, i18n("Configure Cervisia"),
      KDialogBase::Ok | KDialogBase::Cancel | KDialogBase::Help,
      KDialogBase::Ok,
      parent, name, true)
{
    config = conf;

    // open cvs DCOP service configuration file
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
    // read entries from cvs DCOP service configuration
    serviceConfig->setGroup("General");
    cvspathedit->setURL(serviceConfig->readPathEntry("CVSPath", "cvs"));
    m_advancedPage->kcfg_Compression->setValue(serviceConfig->readNumEntry(
                                                   "Compression", 0));
    m_advancedPage->kcfg_UseSshAgent->setChecked(serviceConfig->readBoolEntry(
                                                   "UseSshAgent", false));

    config->setGroup("General");
    m_advancedPage->kcfg_Timeout->setValue(CervisiaSettings::timeout());
    usernameedit->setText(config->readEntry("Username", Cervisia::UserName()));

    contextedit->setValue((int)config->readUnsignedNumEntry("ContextLines", 65535));
    tabwidthedit->setValue((int)config->readUnsignedNumEntry("TabWidth", 8));
    diffoptedit->setText(config->readEntry("DiffOptions"));
    extdiffedit->setURL(config->readPathEntry("ExternalDiff"));
    remotestatusbox->setChecked(config->readBoolEntry("StatusForRemoteRepos", false));
    localstatusbox->setChecked(config->readBoolEntry("StatusForLocalRepos", false));

    // read configuration for look and feel page
    config->setGroup("LookAndFeel");
    m_protocolFontBox->setFont(config->readFontEntry("ProtocolFont"));
    m_annotateFontBox->setFont(config->readFontEntry("AnnotateFont"));
    m_diffFontBox->setFont(config->readFontEntry("DiffFont"));
    m_changelogFontBox->setFont(config->readFontEntry("ChangeLogFont"));
    m_splitterBox->setChecked(config->readBoolEntry("SplitHorizontally",true));

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
    // write entries to cvs DCOP service configuration
    serviceConfig->setGroup("General");
    serviceConfig->writePathEntry("CVSPath", cvspathedit->url());
    serviceConfig->writeEntry("Compression",
        m_advancedPage->kcfg_Compression->value());
    serviceConfig->writeEntry("UseSshAgent", 
        m_advancedPage->kcfg_UseSshAgent->isChecked());

    // write to disk so other services can reparse the configuration
    serviceConfig->sync();

    config->setGroup("General");
    CervisiaSettings::setTimeout(m_advancedPage->kcfg_Timeout->value());
    config->writeEntry("Username", usernameedit->text());

    config->writePathEntry("ExternalDiff", extdiffedit->url());

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
    TQWidgetListIt it(*TQApplication::allWidgets());
    for (; it.current(); ++it)
        {
            TQWidget *w = it.current();
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
    KDialogBase::done(res);
    delete this;
}


/*
 * Create a page for the general options
 */
void SettingsDialog::addGeneralPage()
{
    TQFrame* generalPage = addPage(i18n("General"), TQString::null,
                                  LoadIcon("misc"));
    TQVBoxLayout* layout = new TQVBoxLayout(generalPage, 0, KDialog::spacingHint());

    TQLabel *usernamelabel = new TQLabel( i18n("&User name for the change log editor:"), generalPage );
    usernameedit = new KLineEdit(generalPage);
    usernameedit->setFocus();
    usernamelabel->setBuddy(usernameedit);

    layout->addWidget(usernamelabel);
    layout->addWidget(usernameedit);

    TQLabel *cvspathlabel = new TQLabel( i18n("&Path to CVS executable, or 'cvs':"), generalPage );
    cvspathedit = new KURLRequester(generalPage);
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
    TQGrid *diffPage = addGridPage(2, TQGrid::Horizontal, i18n("Diff Viewer"),
                                  TQString::null, LoadIcon("vcs_diff"));

    TQLabel *contextlabel = new TQLabel( i18n("&Number of context lines in diff dialog:"), diffPage );
    contextedit = new KIntNumInput( 0, diffPage );
    contextedit->setRange(0, 65535, 1, false);
    contextlabel->setBuddy(contextedit);

    TQLabel *diffoptlabel = new TQLabel(i18n("Additional &options for cvs diff:"), diffPage);
    diffoptedit = new KLineEdit(diffPage);
    diffoptlabel->setBuddy(diffoptedit);

    TQLabel *tabwidthlabel = new TQLabel(i18n("Tab &width in diff dialog:"), diffPage);
    tabwidthedit = new KIntNumInput(0, diffPage);
    tabwidthedit->setRange(1, 16, 1, false);
    tabwidthlabel->setBuddy(tabwidthedit);

    TQLabel *extdifflabel = new TQLabel(i18n("External diff &frontend:"), diffPage);
    extdiffedit = new KURLRequester(diffPage);
    extdifflabel->setBuddy(extdiffedit);

    // dummy widget to take up the vertical space
    new TQWidget(diffPage);
}


/*
 * Create a page for the status options
 */
void SettingsDialog::addStatusPage()
{
    TQVBox* statusPage = addVBoxPage(i18n("Status"), TQString::null,
                                    LoadIcon("fork"));

    remotestatusbox = new TQCheckBox(i18n("When opening a sandbox from a &remote repository,\n"
                                         "start a File->Status command automatically"), statusPage);
    localstatusbox = new TQCheckBox(i18n("When opening a sandbox from a &local repository,\n"
                                        "start a File->Status command automatically"), statusPage);

    // dummy widget to take up the vertical space
    new TQWidget(statusPage);
}


/*
 * Create a page for the advanced options
 */
void SettingsDialog::addAdvancedPage()
{
    TQVBox* frame = addVBoxPage(i18n("Advanced"), TQString::null,
                               LoadIcon("configure"));

    m_advancedPage = new AdvancedPage(frame);
    m_advancedPage->kcfg_Timeout->setRange(0, 50000, 100, false);
    m_advancedPage->kcfg_Compression->setRange(0, 9, 1, false);
}


/*
 * Create a page for the look & feel options
 */
void SettingsDialog::addLookAndFeelPage()
{
    TQVBox* lookPage = addVBoxPage(i18n("Appearance"), TQString::null,
                                  LoadIcon("looknfeel"));

    TQGroupBox* fontGroupBox = new TQGroupBox(4, Qt::Vertical, i18n("Fonts"),
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

    TQGroupBox* colorGroupBox = new TQGroupBox(4, Qt::Horizontal,
                                             i18n("Colors"), lookPage);
    colorGroupBox->setColumns(4);
    colorGroupBox->setInsideSpacing(KDialog::spacingHint());

    TQLabel* conflictLabel = new TQLabel(i18n("Conflict:"), colorGroupBox);
    m_conflictButton      = new KColorButton(colorGroupBox);
    conflictLabel->setBuddy(m_conflictButton);

    TQLabel* diffChangeLabel = new TQLabel(i18n("Diff change:"), colorGroupBox);
    m_diffChangeButton      = new KColorButton(colorGroupBox);
    diffChangeLabel->setBuddy(m_diffChangeButton);

    TQLabel* localChangeLabel = new TQLabel(i18n("Local change:"), colorGroupBox);
    m_localChangeButton      = new KColorButton(colorGroupBox);
    localChangeLabel->setBuddy(m_localChangeButton);

    TQLabel* diffInsertLabel = new TQLabel(i18n("Diff insertion:"), colorGroupBox);
    m_diffInsertButton      = new KColorButton(colorGroupBox);
    diffInsertLabel->setBuddy(m_diffInsertButton);

    TQLabel* remoteChangeLabel = new TQLabel(i18n("Remote change:"), colorGroupBox);
    m_remoteChangeButton      = new KColorButton(colorGroupBox);
    remoteChangeLabel->setBuddy( m_remoteChangeButton );

    TQLabel* diffDeleteLabel = new TQLabel(i18n("Diff deletion:"), colorGroupBox);
    m_diffDeleteButton      = new KColorButton(colorGroupBox);
    diffDeleteLabel->setBuddy(m_diffDeleteButton);

    TQLabel* notInCvsLabel = new TQLabel(i18n("Not in cvs:"), colorGroupBox);
    m_notInCvsButton      = new KColorButton(colorGroupBox);
    notInCvsLabel->setBuddy(m_notInCvsButton);

    m_splitterBox = new TQCheckBox(i18n("Split main window &horizontally"), lookPage);
}

#include "settingsdlg.moc"


// Local Variables:
// c-basic-offset: 4
// End:
