/*
 *  Copyright (C) 1999-2002 Bernd Gehrmann
 *                          bernd@mail.berlios.de
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


#include "updatedlg.h"

#include <tqbuttongroup.h>
#include <tqcombobox.h>
#include <tqlayout.h>
#include <tqpushbutton.h>
#include <tqradiobutton.h>
#include <tqstyle.h>
#include <klineedit.h>
#include <klocale.h>

#include "misc.h"
#include "cvsservice_stub.h"


UpdateDialog::UpdateDialog(CvsService_stub* service,
                           TQWidget *parent, const char *name)
    : KDialogBase(parent, name, true, i18n("CVS Update"),
                  Ok | Cancel, Ok, true),
      cvsService(service)
{
    int const iComboBoxMinWidth(40 * fontMetrics().width('0'));
    int const iWidgetIndent(tqstyle().tqpixelMetric(TQStyle::PM_ExclusiveIndicatorWidth, 0) + 6);

    TQFrame* mainWidget = makeMainWidget();

    TQBoxLayout *tqlayout = new TQVBoxLayout(mainWidget, 0, spacingHint());

    bybranch_button = new TQRadioButton(i18n("Update to &branch: "), mainWidget);
    bybranch_button->setChecked(true);
    tqlayout->addWidget(bybranch_button);

    branch_combo = new TQComboBox(true, mainWidget);
    branch_combo->setMinimumWidth(iComboBoxMinWidth);
    
    branch_button = new TQPushButton(i18n("Fetch &List"), mainWidget);
    connect( branch_button, TQT_SIGNAL(clicked()),
             this, TQT_SLOT(branchButtonClicked()) );
            
    TQBoxLayout *branchedit_layout = new TQHBoxLayout(tqlayout);
    branchedit_layout->addSpacing(iWidgetIndent);
    branchedit_layout->addWidget(branch_combo);
    branchedit_layout->addWidget(branch_button);
    
    bytag_button = new TQRadioButton(i18n("Update to &tag: "), mainWidget);
    tqlayout->addWidget(bytag_button);

    tag_combo = new TQComboBox(true, mainWidget);
    tag_combo->setMinimumWidth(iComboBoxMinWidth);
    
    tag_button = new TQPushButton(i18n("Fetch L&ist"), mainWidget);
    connect( tag_button, TQT_SIGNAL(clicked()),
             this, TQT_SLOT(tagButtonClicked()) );
            
    TQBoxLayout *tagedit_layout = new TQHBoxLayout(tqlayout);
    tagedit_layout->addSpacing(iWidgetIndent);
    tagedit_layout->addWidget(tag_combo);
    tagedit_layout->addWidget(tag_button);
    
    bydate_button = new TQRadioButton(i18n("Update to &date ('yyyy-mm-dd'):"), mainWidget);
    tqlayout->addWidget(bydate_button);

    date_edit = new KLineEdit(mainWidget);

    TQBoxLayout *dateedit_layout = new TQHBoxLayout(tqlayout);
    dateedit_layout->addSpacing(iWidgetIndent);
    dateedit_layout->addWidget(date_edit);

    TQButtonGroup* group = new TQButtonGroup(mainWidget);
    group->hide();
    group->insert(bytag_button);
    group->insert(bybranch_button);
    group->insert(bydate_button);
    connect( group, TQT_SIGNAL(clicked(int)),
             this, TQT_SLOT(toggled()) );

    // dis-/enable the widgets
    toggled();
}


bool UpdateDialog::byTag() const
{
    return bybranch_button->isChecked() || bytag_button->isChecked();
}


TQString UpdateDialog::tag() const
{
    return bybranch_button->isChecked()
        ? branch_combo->currentText()
        : tag_combo->currentText();
}


TQString UpdateDialog::date() const
{
    return date_edit->text();
}


void UpdateDialog::tagButtonClicked()
{
    tag_combo->clear();
    tag_combo->insertStringList(::fetchTags(cvsService, this));
}


void UpdateDialog::branchButtonClicked()
{
    branch_combo->clear();
    branch_combo->insertStringList(::fetchBranches(cvsService, this));
}


void UpdateDialog::toggled()
{
    bool bytag = bytag_button->isChecked();
    tag_combo->setEnabled(bytag);
    tag_button->setEnabled(bytag);
    if (bytag)
        tag_combo->setFocus();

    bool bybranch = bybranch_button->isChecked();
    branch_combo->setEnabled(bybranch);
    branch_button->setEnabled(bybranch);
    if (bybranch)
        branch_combo->setFocus();

    bool bydate = bydate_button->isChecked();
    date_edit->setEnabled(bydate);
    if (bydate)
        date_edit->setFocus();
}

#include "updatedlg.moc"


// Local Variables:
// c-basic-offset: 4
// End:
