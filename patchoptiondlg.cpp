/*
 *  Copyright (C) 2004 Christian Loose <christian.loose@kdemail.net>
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

#include "patchoptiondlg.h"
using Cervisia::PatchOptionDialog;

#include <tqcheckbox.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqradiobutton.h>
#include <tqvbuttongroup.h>
#include <knuminput.h>
#include <klocale.h>


PatchOptionDialog::PatchOptionDialog(TQWidget* parent, const char* name)
    : KDialogBase(parent, name, true/*modal*/, TQString(),
                  Ok | Cancel | Help, Ok, true/*separator*/)
{
    TQFrame* mainWidget = makeMainWidget();
    TQBoxLayout* topLayout = new TQVBoxLayout(mainWidget, 0, spacingHint());

    m_formatBtnGroup = new TQVButtonGroup(i18n("Output Format"), mainWidget, "");
    topLayout->addWidget(m_formatBtnGroup);

    connect(m_formatBtnGroup, TQT_SIGNAL(clicked(int)),
            this,             TQT_SLOT(formatChanged(int)));

    new TQRadioButton(i18n( "Context" ), m_formatBtnGroup);
    new TQRadioButton(i18n( "Normal" ), m_formatBtnGroup);
    TQRadioButton* unifiedFormatBtn = new TQRadioButton(i18n( "Unified" ), m_formatBtnGroup);
    unifiedFormatBtn->setChecked(true);

    TQLabel* contextLinesLbl = new TQLabel(i18n("&Number of context lines:"),
                                         mainWidget);
    m_contextLines = new KIntNumInput(3, mainWidget);
    m_contextLines->setRange(2, 65535, 1, false);
    contextLinesLbl->setBuddy(m_contextLines);

    TQBoxLayout* contextLinesLayout = new TQHBoxLayout(topLayout);
    contextLinesLayout->addWidget(contextLinesLbl);
    contextLinesLayout->addWidget(m_contextLines);

    TQVButtonGroup* ignoreBtnGroup = new TQVButtonGroup(i18n("Ignore Options"), mainWidget);
    topLayout->addWidget(ignoreBtnGroup);

    m_blankLineChk = new TQCheckBox(i18n("Ignore added or removed empty lines"),
                                   ignoreBtnGroup);
    m_spaceChangeChk = new TQCheckBox(i18n("Ignore changes in the amount of whitespace"),
                                     ignoreBtnGroup);
    m_allSpaceChk = new TQCheckBox(i18n("Ignore all whitespace"), ignoreBtnGroup);
    m_caseChangesChk = new TQCheckBox(i18n("Ignore changes in case"), ignoreBtnGroup);
}


PatchOptionDialog::~PatchOptionDialog()
{
}


TQString PatchOptionDialog::diffOptions() const
{
    TQString options;

    if( m_blankLineChk->isChecked() )
        options += " -B ";

    if( m_spaceChangeChk->isChecked() )
        options += " -b ";

    if( m_allSpaceChk->isChecked() )
        options += " -w ";

    if( m_caseChangesChk->isChecked() )
        options += " -i ";

    return options;
}


TQString PatchOptionDialog::formatOption() const
{
    switch( m_formatBtnGroup->selectedId() )
    {
        case 0: return "-C " + TQString::number(m_contextLines->value());
        case 1: return "";
        case 2: return "-U " + TQString::number(m_contextLines->value());
    }

    return "";
}


void PatchOptionDialog::formatChanged(int buttonId)
{
    bool enabled = ( buttonId == 0 || buttonId == 2 );
    m_contextLines->setEnabled(enabled);
}

#include "patchoptiondlg.moc"
