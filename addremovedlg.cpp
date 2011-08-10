/*
 *  Copyright (C) 1999-2002 Bernd Gehrmann
 *                          bernd@mail.berlios.de
 *  Copyright (c) 2003 Christian Loose <christian.loose@hamburg.de>
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

#include "addremovedlg.h"

#include <tqfileinfo.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqlistbox.h>
#include <tqstringlist.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <klocale.h>


AddRemoveDialog::AddRemoveDialog(ActionType action, TQWidget* tqparent, const char* name)
    : KDialogBase(tqparent, name, true, TQString(),
                  Ok | Cancel | Help, Ok, true)
{
    setCaption( (action==Add)?       i18n("CVS Add") :
                (action==AddBinary)? i18n("CVS Add Binary") :
                                     i18n("CVS Remove") );

    TQFrame* mainWidget = makeMainWidget();

    TQBoxLayout *tqlayout = new TQVBoxLayout(mainWidget, 0, spacingHint());

    TQLabel *textlabel = new TQLabel
        ( (action==Add)?       i18n("Add the following files to the repository:") :
          (action==AddBinary)? i18n("Add the following binary files to the repository:") :
                               i18n("Remove the following files from the repository:") ,
          mainWidget );
    tqlayout->addWidget(textlabel);

    m_listBox = new TQListBox(mainWidget);
    m_listBox->setSelectionMode(TQListBox::NoSelection);
    tqlayout->addWidget(m_listBox, 5);

    // Add warning message to dialog when user wants to remove a file
    if (action==Remove)
    {
        TQBoxLayout *warningLayout = new TQHBoxLayout;

        TQLabel *warningIcon = new TQLabel(mainWidget);
        KIconLoader *loader = kapp->iconLoader();
        warningIcon->setPixmap(loader->loadIcon("messagebox_warning", KIcon::NoGroup,
                                                KIcon::SizeMedium, KIcon::DefaultState,
                                                0, true));
        warningLayout->addWidget(warningIcon);

        TQLabel *warningText = new TQLabel(i18n("This will also remove the files from "
                                              "your local working copy."), mainWidget);
        warningLayout->addWidget(warningText);

        tqlayout->addSpacing(5);
        tqlayout->addLayout(warningLayout);
        tqlayout->addSpacing(5);
    }

    if( action == Remove )
        setHelp("removingfiles");
    else
        setHelp("addingfiles");
}


void AddRemoveDialog::setFileList(const TQStringList& files)
{
    // the dot for the root directory is hard to see, so
    // we convert it to the absolut path
    if( files.find(".") != files.end() )
    {
        TQStringList copy(files);
        int idx = copy.findIndex(".");
        copy[idx] = TQFileInfo(".").absFilePath();

        m_listBox->insertStringList(copy);
    }
    else
        m_listBox->insertStringList(files);
}


// kate: space-indent on; indent-width 4; replace-tabs on;
