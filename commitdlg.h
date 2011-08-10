/* 
 *  Copyright (C) 1999-2002 Bernd Gehrmann
 *                          bernd@mail.berlios.de
 *  Copyright (c) 2003-2005 Christian Loose <christian.loose@kdemail.net>
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


#ifndef COMMITDLG_H
#define COMMITDLG_H

#include <tqstringlist.h>
#include <kdialogbase.h>

namespace Cervisia { class LogMessageEdit; }

class TQComboBox;
class TQCheckBox;
class KListView;
class KConfig;
class CvsService_stub;


class CommitDialog : public KDialogBase
{
    Q_OBJECT
  TQ_OBJECT

public:
    CommitDialog( KConfig& cfg, CvsService_stub* service, TQWidget *parent=0, 
                  const char *name=0 );

    virtual ~CommitDialog();

    void setFileList(const TQStringList &list);
    TQStringList fileList() const;
    void setLogMessage(const TQString &msg);
    TQString logMessage() const;
    void setLogHistory(const TQStringList &list);

private slots:
    void comboActivated(int);
    void fileSelected(TQListViewItem* item);
    void fileHighlighted();
    void diffClicked();
    void useTemplateClicked();

private:
    void showDiffDialog(const TQString& fileName);
    void checkForTemplateFile();
    void addTemplateText();
    void removeTemplateText();

    KListView* m_fileList;
    Cervisia::LogMessageEdit* edit;
    TQComboBox *combo;
    TQStringList commits;
    int current_index;
    TQString current_text;
    int highlightedFile;

    TQCheckBox* m_useTemplateChk;
    TQString    m_templateText;

    KConfig&            partConfig;
    CvsService_stub*    cvsService;     // for diff dialog
};

#endif


// Local Variables:
// c-basic-offset: 4
// End:
