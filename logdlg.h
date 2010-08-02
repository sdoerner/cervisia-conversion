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


#ifndef LOGDLG_H
#define LOGDLG_H

#include <kdialogbase.h>

#include "loginfo.h"

#include <tqptrlist.h>


class LogListView;
class LogTreeView;
class LogPlainView;

class KConfig;

class TQComboBox;
class TQLabel;
class TQTabWidget;
class TQTextEdit;
class CvsService_stub;

class LogDialogTagInfo
{
public:
    TQString rev;
    TQString tag;
    TQString branchpoint;
};


class LogDialog : public KDialogBase
{
    Q_OBJECT

public:
    explicit LogDialog( KConfig& cfg, TQWidget *parent=0, const char *name=0 );

    virtual ~LogDialog();

    bool parseCvsLog(CvsService_stub* service, const TQString& fileName);

protected slots:
    void slotOk();
    void slotApply();

private slots:
    void findClicked();
    void diffClicked();
    void annotateClicked();
    void revisionSelected(TQString rev, bool rmb);
    void tagASelected(int n);
    void tagBSelected(int n);
    void tabChanged(TQWidget* w);

private:
    void tagSelected(LogDialogTagInfo* tag, bool rmb);
    void updateButtons();

    TQString filename;
    TQPtrList<Cervisia::LogInfo> items;
    TQPtrList<LogDialogTagInfo> tags;
    TQString selectionA;
    TQString selectionB;
    LogTreeView *tree;
    LogListView *list;
    LogPlainView *plain;
    TQTabWidget *tabWidget;
    TQLabel *revbox[2];
    TQLabel *authorbox[2];
    TQLabel *datebox[2];
    TQTextEdit *commentbox[2];
    TQTextEdit *tagsbox[2];
    TQComboBox *tagcombo[2];

    CvsService_stub* cvsService;
    KConfig&         partConfig;
};

#endif


// Local Variables:
// c-basic-offset: 4
// End:
