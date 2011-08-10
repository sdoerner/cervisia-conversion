/*
 *  Copyright (C) 1999-2002 Bernd Gehrmann
 *                          bernd@mail.berlios.de
 *  Copyright (c) 2002-2005 Christian Loose <christian.loose@kdemail.net>
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

#include "commitdlg.h"

#include <tqcombobox.h>
#include <tqcheckbox.h>
#include <tqdir.h>
#include <tqfileinfo.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqheader.h>
#include <klistview.h>
#include <kconfig.h>
#include <klocale.h>

#include "cvsservice_stub.h"
#include "logmessageedit.h"
#include "diffdlg.h"


class CommitListItem : public TQCheckListItem
{
public:
    CommitListItem(TQListView* parent, const TQString& text, const TQString fileName)
        : TQCheckListItem(parent, text, TQCheckListItem::CheckBox)
        , m_fileName(fileName)
    {
    }

    TQString fileName() const { return m_fileName; }

private:
    TQString m_fileName;
};


CommitDialog::CommitDialog(KConfig& cfg, CvsService_stub* service,
                           TQWidget *parent, const char *name)
    : KDialogBase(parent, name, true, i18n("CVS Commit"),
                  Ok | Cancel | Help | User1, Ok, true)
    , partConfig(cfg)
    , cvsService(service)
{
    TQFrame* mainWidget = makeMainWidget();

    TQBoxLayout *tqlayout = new TQVBoxLayout(mainWidget, 0, spacingHint());

    TQLabel *textlabel = new TQLabel( i18n("Commit the following &files:"), mainWidget );
    tqlayout->addWidget(textlabel);

    m_fileList = new KListView(mainWidget);
    m_fileList->addColumn("");
    m_fileList->setFullWidth(true);
    m_fileList->header()->hide();
    textlabel->setBuddy(m_fileList);
    connect( m_fileList, TQT_SIGNAL(doubleClicked(TQListViewItem*)),
             this, TQT_SLOT(fileSelected(TQListViewItem*)));
    connect( m_fileList, TQT_SIGNAL(selectionChanged()),
             this, TQT_SLOT(fileHighlighted()) );
    tqlayout->addWidget(m_fileList, 5);

    TQLabel *archivelabel = new TQLabel(i18n("Older &messages:"), mainWidget);
    tqlayout->addWidget(archivelabel);

    combo = new TQComboBox(mainWidget);
    archivelabel->setBuddy(combo);
    connect( combo, TQT_SIGNAL(activated(int)), this, TQT_SLOT(comboActivated(int)) );
    // make sure that combobox is smaller than the screen
    combo->tqsetSizePolicy(TQSizePolicy(TQSizePolicy::Preferred, TQSizePolicy::Fixed));
    tqlayout->addWidget(combo);

    TQLabel *messagelabel = new TQLabel(i18n("&Log message:"), mainWidget);
    tqlayout->addWidget(messagelabel);

    edit = new Cervisia::LogMessageEdit(mainWidget);
    messagelabel->setBuddy(edit);
    edit->setCheckSpellingEnabled(true);
    edit->setFocus();
    edit->setMinimumSize(400, 100);
    tqlayout->addWidget(edit, 10);

    m_useTemplateChk = new TQCheckBox(i18n("Use log message &template"), mainWidget);
    tqlayout->addWidget(m_useTemplateChk);
    connect( m_useTemplateChk, TQT_SIGNAL(clicked()), this, TQT_SLOT(useTemplateClicked()) );

    checkForTemplateFile();

    setButtonGuiItem(User1, KGuiItem(i18n("&Diff"), "vcs_diff"));
    enableButton(User1, false);
    connect( this, TQT_SIGNAL(user1Clicked()),
             this, TQT_SLOT(diffClicked()) );

    setHelp("commitingfiles");

    TQSize size = configDialogSize(partConfig, "CommitDialog");
    resize(size);
}


CommitDialog::~CommitDialog()
{
    saveDialogSize(partConfig, "CommitDialog");

    KConfigGroupSaver cs(&partConfig, "CommitDialog");
    partConfig.writeEntry("UseTemplate", m_useTemplateChk->isChecked());
}


void CommitDialog::setFileList(const TQStringList &list)
{
    TQString currentDirName = TQFileInfo(TQChar('.')).absFilePath();

    TQStringList::ConstIterator it = list.begin();
    for( ; it != list.end(); ++it )
    {
        // the dot for the root directory is hard to see, so
        // we convert it to the absolut path
        TQString text = (*it != "." ? *it : currentDirName);

        edit->compObj()->addItem(text);
        CommitListItem* item = new CommitListItem(m_fileList, text, *it);
        item->setOn(true);
    }
}


TQStringList CommitDialog::fileList() const
{
    TQStringList files;

    TQListViewItemIterator it(m_fileList, TQListViewItemIterator::Checked);
    for( ; it.current(); ++it )
    {
        CommitListItem* item = static_cast<CommitListItem*>(it.current());
        files.append(item->fileName());
    }

    return files;
}


void CommitDialog::setLogMessage(const TQString &msg)
{
    edit->setText(msg);

    if( m_useTemplateChk->isChecked() )
        addTemplateText();
}


TQString CommitDialog::logMessage() const
{
    return edit->text();
}


void CommitDialog::setLogHistory(const TQStringList &list)
{
    commits = list;

    combo->insertItem(i18n("Current"));

    for ( TQStringList::ConstIterator it = list.begin();
          it != list.end(); ++it )
        {
            if( (*it).isEmpty() )
                continue;

            TQString txt = *it;
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


void CommitDialog::fileSelected(TQListViewItem* item)
{
    // double click on empty space?
    if( !item )
        return;

    showDiffDialog(item->text(0));
}


void CommitDialog::fileHighlighted()
{
    bool isItemSelected = (m_fileList->selectedItem() != 0);
    enableButton(User1, isItemSelected);
}


void CommitDialog::diffClicked()
{
    TQListViewItem* item = m_fileList->selectedItem();
    if( !item )
        return;

    showDiffDialog(item->text(0));
}


void CommitDialog::showDiffDialog(const TQString& fileName)
{
    DiffDialog *l = new DiffDialog(partConfig, this, "diffdialog");

    // disable diff button so user doesn't open the same diff several times (#83018)
    enableButton(User1, false);

    if (l->parseCvsDiff(cvsService, fileName, "", ""))
        l->show();
    else
        delete l;

    // re-enable diff button
    enableButton(User1, true);
}


void CommitDialog::useTemplateClicked()
{
    if( m_useTemplateChk->isChecked() )
    {
        addTemplateText();
    }
    else
    {
        removeTemplateText();
    }
}


void CommitDialog::checkForTemplateFile()
{
    TQString filename = TQDir::current().absPath() + "/CVS/Template";
    if( TQFile::exists(filename) )
    {
        TQFile f(filename);
        if( f.open(IO_ReadOnly) )
        {
            TQTextStream stream(&f);
            m_templateText = stream.read();
            f.close();

            m_useTemplateChk->setEnabled(true);
            KConfigGroupSaver cs(&partConfig, "CommitDialog");
            bool check = partConfig.readBoolEntry("UseTemplate", true);
            m_useTemplateChk->setChecked(check);

            addTemplateText();
        }
        else
        {
            m_useTemplateChk->setEnabled(false);
        }
    }
    else
    {
        m_useTemplateChk->setEnabled(false);
    }
}


void CommitDialog::addTemplateText()
{
    edit->append(m_templateText);
    edit->moveCursor(TQTextEdit::MoveHome, false);
    edit->ensureCursorVisible();
}


void CommitDialog::removeTemplateText()
{
    edit->setText(edit->text().remove(m_templateText));
}


#include "commitdlg.moc"


// Local Variables:
// c-basic-offset: 4
// End:
