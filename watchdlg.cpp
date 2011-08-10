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


#include "watchdlg.h"

#include <tqbuttongroup.h>
#include <tqcheckbox.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqradiobutton.h>
#include <klocale.h>


WatchDialog::WatchDialog(ActionType action, TQWidget *parent, const char *name)
    : KDialogBase(parent, name, true, TQString(),
                  Ok | Cancel | Help, Ok, true)
{
    setCaption( (action==Add)? i18n("CVS Watch Add") : i18n("CVS Watch Remove") );

    TQFrame* mainWidget = makeMainWidget();

    TQBoxLayout *tqlayout = new TQVBoxLayout(mainWidget, 0, spacingHint());

    TQLabel *textlabel = new TQLabel
	( (action==Add)? i18n("Add watches for the following events:")
          :  i18n("Remove watches for the following events:"), mainWidget );
    tqlayout->addWidget(textlabel, 0);

    all_button = new TQRadioButton(i18n("&All"), mainWidget);
    all_button->setFocus();
    all_button->setChecked(true);
    tqlayout->addWidget(all_button);
    
    only_button = new TQRadioButton(i18n("&Only:"), mainWidget);
    tqlayout->addWidget(only_button);

    TQGridLayout *eventstqlayout = new TQGridLayout(tqlayout);
    eventstqlayout->addColSpacing(0, 20);
    eventstqlayout->setColStretch(0, 0);
    eventstqlayout->setColStretch(1, 1);
    
    commitbox = new TQCheckBox(i18n("&Commits"), mainWidget);
    commitbox->setEnabled(false);
    eventstqlayout->addWidget(commitbox, 0, 1);
    
    editbox = new TQCheckBox(i18n("&Edits"), mainWidget);
    editbox->setEnabled(false);
    eventstqlayout->addWidget(editbox, 1, 1);

    uneditbox = new TQCheckBox(i18n("&Unedits"), mainWidget);
    uneditbox->setEnabled(false);
    eventstqlayout->addWidget(uneditbox, 2, 1);

    TQButtonGroup* group = new TQButtonGroup(mainWidget);
    group->hide();
    group->insert(all_button);
    group->insert(only_button);

    connect( only_button, TQT_SIGNAL(toggled(bool)),
             commitbox, TQT_SLOT(setEnabled(bool)) );
    connect( only_button, TQT_SIGNAL(toggled(bool)),
             editbox, TQT_SLOT(setEnabled(bool)) );
    connect( only_button, TQT_SIGNAL(toggled(bool)),
             uneditbox, TQT_SLOT(setEnabled(bool)) );

    setHelp("watches");
}


WatchDialog::Events WatchDialog::events() const
{
    Events res = None;
    if (all_button->isChecked())
        res = All;
    else
        {
            if (commitbox->isChecked())
                res = Events(res | Commits);
            if (editbox->isChecked())
                res = Events(res | Edits);
            if (uneditbox->isChecked())
                res = Events(res | Unedits);
        }
    return res;
}


// Local Variables:
// c-basic-offset: 4
// End:
