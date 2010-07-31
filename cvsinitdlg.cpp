/*
 *  Copyright (c) 2004 Christian Loose <christian.loose@kdemail.net>
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


#include "cvsinitdlg.h"

#include <tqlabel.h>
#include <tqlayout.h>
#include <tqpushbutton.h>

#include <kfiledialog.h>
#include <klineedit.h>
#include <klocale.h>
#include <kurlcompletion.h>


using Cervisia::CvsInitDialog;


CvsInitDialog::CvsInitDialog(TQWidget* parent, const char* name)
    : KDialogBase(parent, name, true, i18n("Create New Repository (cvs init)"),
                  Ok | Cancel, Ok, true)
{
    TQFrame* mainWidget = makeMainWidget();
    TQVBoxLayout* mainLayout = new TQVBoxLayout(mainWidget, 0, spacingHint());

    TQLabel* dirLabel = new TQLabel(i18n("Repository folder:"), mainWidget);
    mainLayout->addWidget(dirLabel);

    TQHBoxLayout* dirLayout = new TQHBoxLayout(mainLayout);
     
    m_directoryEdit = new KLineEdit(mainWidget);
    m_directoryEdit->setFocus();
        
    KURLCompletion* comp = new KURLCompletion();
    m_directoryEdit->setCompletionObject(comp);
    m_directoryEdit->setAutoDeleteCompletionObject(true);

    dirLabel->setBuddy(m_directoryEdit);
    dirLayout->addWidget(m_directoryEdit); 
    
    TQPushButton* dirButton = new TQPushButton("...", mainWidget);
    dirButton->setFixedWidth(30);
    dirLayout->addWidget(dirButton);
    
    connect( dirButton, TQT_SIGNAL(clicked()),
             this,      TQT_SLOT(dirButtonClicked()) );
    connect( m_directoryEdit, TQT_SIGNAL(textChanged(const TQString&)),
             this,            TQT_SLOT(lineEditTextChanged(const TQString&)));
             
    enableButton(Ok, false);

    setMinimumWidth(350);
}


TQString CvsInitDialog::directory() const
{
    return m_directoryEdit->text();
}


void CvsInitDialog::dirButtonClicked()
{
    TQString dir = KFileDialog::getExistingDirectory(m_directoryEdit->text());
    if( !dir.isEmpty() )
        m_directoryEdit->setText(dir);
}


void CvsInitDialog::lineEditTextChanged(const TQString& text)
{
    enableButton(Ok, !text.stripWhiteSpace().isEmpty());
}
    
#include "cvsinitdlg.moc"
