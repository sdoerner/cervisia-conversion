/* 
 *  Copyright (C) 1999-2002 Bernd Gehrmann
 *                          bernd@mail.berlios.de
 *  Copyright (c) 2003-2004 Christian Loose <christian.loose@hamburg.de>
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


#ifndef RESOLVEDLG_H
#define RESOLVEDLG_H


#include <kdialogbase.h>

#include <tqptrlist.h>
#include "diffview.h"


class TQLabel;
class TQTextCodec;
class KConfig;
class ResolveItem;


class ResolveDialog : public KDialogBase
{
    Q_OBJECT

public:
    enum ChooseType { ChA, ChB, ChAB, ChBA, ChEdit };

    explicit ResolveDialog( KConfig& cfg, TQWidget *parent=0, const char *name=0 );
    virtual ~ResolveDialog();

    bool parseFile(const TQString &name);

protected:
    virtual void keyPressEvent(TQKeyEvent *e);

private slots:
    void backClicked();
    void forwClicked();
    void aClicked();
    void bClicked();
    void abClicked();
    void baClicked();
    void editClicked();
    void saveClicked();
    void saveAsClicked();
    
private:
    void updateNofN();
    void updateHighlight(int newitem);
    void choose(ChooseType ch);
    void chooseEdit();
    void saveFile(const TQString &name);
    TQString readFile();
    void addToMergeAndVersionA(const TQString& line, DiffView::DiffType type, 
                               int& lineNo);
    void addToVersionB(const TQString& line, DiffView::DiffType type, int& lineNo);
    void updateMergedVersion(ResolveItem* item, ChooseType chosen);
    TQString contentVersionA(const ResolveItem *item);
    TQString contentVersionB(const ResolveItem *item);
    
    TQLabel *nofnlabel;
    TQPushButton *backbutton, *forwbutton;
    TQPushButton *abutton, *bbutton, *abbutton, *babutton, *editbutton;
    DiffView *diff1, *diff2, *merge;

    TQPtrList<ResolveItem> items;
    TQString fname;
    TQTextCodec *fcodec;
    int markeditem;
    KConfig& partConfig;
    
    TQString    m_contentMergedVersion;
};


#endif


// Local Variables:
// c-basic-offset: 4
// End:
