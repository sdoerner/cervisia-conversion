/* 
 *  Copyright (C) 1999-2002 Bernd Gehrmann
 *                          bernd@mail.berlios.de
 *
 * This program may be distributed under the terms of the Q Public
 * License as defined by Trolltech AS of Norway and appearing in the
 * file LICENSE.QPL included in the packaging of this file.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "commitdlg.h"

#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlistbox.h>
#include <qmultilineedit.h>
#include <kconfig.h>
#include <klocale.h>

#include "diffdlg.h"


CommitDialog::Options *CommitDialog::options = 0;


CommitDialog::CommitDialog(ActionType action, QWidget *parent, const char *name)
    : KDialogBase(parent, name, true, QString::null,
                  Ok | Cancel, Ok, true),
      edit(0)
{
    setCaption( (action==Add)?       i18n("CVS Add") :
                (action==AddBinary)? i18n("CVS Add Binary") :
                (action==Remove)?    i18n("CVS Remove") :
                                     i18n("CVS Commit") );

    QFrame* mainWidget = makeMainWidget();

    QBoxLayout *layout = new QVBoxLayout(mainWidget, 0, spacingHint());

    QLabel *textlabel = new QLabel
        ( (action==Add)?       i18n("Add the following files to the repository:") :
          (action==AddBinary)? i18n("Add the following binary files to the repository:") :
          (action==Remove)?    i18n("Remove the following files from the repository:") :
                               i18n("Commit the following &files:"),
          mainWidget );
    layout->addWidget(textlabel);

    listbox = new QListBox(mainWidget);
    textlabel->setBuddy(listbox);
    connect( listbox, SIGNAL(selected(int)), this, SLOT(fileSelected(int)));
    layout->addWidget(listbox, 5);

    if (action == Commit)
        {
            QLabel *archivelabel = new QLabel(i18n("Older &messages:"), mainWidget);
            layout->addWidget(archivelabel);
            
            combo = new QComboBox(mainWidget);
            archivelabel->setBuddy(combo);
            connect( combo, SIGNAL(activated(int)), this, SLOT(comboActivated(int)) );
            // make sure that combobox is smaller than the screen
            combo->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed));
            layout->addWidget(combo);
            
            QLabel *messagelabel = new QLabel(i18n("&Log message:"), mainWidget);
            layout->addWidget(messagelabel);

            edit = new QMultiLineEdit(mainWidget);
            messagelabel->setBuddy(edit);
            edit->setFocus();
            edit->setMinimumSize(400, 100);
            layout->addWidget(edit, 10);
        }
    else
        listbox->setEnabled(false);

    if (options && edit) // Only for commits
        resize(options->size);
}


CommitDialog::~CommitDialog()
{
    if (edit) // Only for commits
        {
            if (!options)
                options = new Options;
            options->size = size();
        }
}


void CommitDialog::setFileList(const QStringList &list)
{
    listbox->insertStringList(list);
}


void CommitDialog::setLogMessage(const QString &msg)
{
    edit->setText(msg);
}


QString CommitDialog::logMessage() const
{
    return edit->text();
}


void CommitDialog::loadOptions(KConfig *config)
{
    if (!config->readEntry("Customized"))
        return;

    options = new Options;
    options->size = config->readSizeEntry("Size");
}


void CommitDialog::saveOptions(KConfig *config)
{
    if (!options)
        return;

    config->writeEntry("Customized", true);
    config->writeEntry("Size", options->size);
}


void CommitDialog::setLogHistory(const QString &sbox, const QString &repo,
                                 const QStringList &list)
{
    sandbox = sbox;
    repository = repo;
    commits = list;

    combo->insertItem(i18n("Current"));

    for ( QStringList::ConstIterator it = list.begin();
          it != list.end(); ++it )
        {
            if( (*it).isEmpty() )
                continue;

            QString txt = *it;
            int index = txt.find('\n', 0);
            if ( index != -1 ) // Fetch first line
            {
                txt = txt.mid(0, index);
                txt += "...";
            }

            combo->insertItem(txt);
        }
}


void CommitDialog::comboActivated(int index)
{
    if (index == current_index)
        return;
    
    if (index == 0) // Handle current text
        edit->setText(current_text);
    else
        {
            if (current_index == 0) // Store current text
                current_text = edit->text();
            
            // Show archived text
            edit->setText(commits[index-1]);
        }
    current_index = index;
}


void CommitDialog::fileSelected(int index)
{
    QListBoxItem *item = listbox->item(index);
    if ( !item )
        return;
    QString filename = item->text();

    DiffDialog *l = new DiffDialog(this, "diffdialog", true);
    if (l->parseCvsDiff(sandbox, repository, filename, "", ""))
        l->show();
    else
        delete l;
}

#include "commitdlg.moc"


// Local Variables:
// c-basic-offset: 4
// End:
