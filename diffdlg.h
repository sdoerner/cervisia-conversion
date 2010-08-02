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


#ifndef DIFFDLG_H
#define DIFFDLG_H

#include <tqptrlist.h>
#include <kdialogbase.h>


class TQLabel;
class TQCheckBox;
class TQComboBox;
class KConfig;
class DiffItem;
class DiffView;
class CvsService_stub;


class DiffDialog : public KDialogBase
{
    Q_OBJECT

public:

    explicit DiffDialog( KConfig& config, TQWidget *parent=0, const char *name=0, 
                         bool modal=false );

    virtual ~DiffDialog();

    bool parseCvsDiff(CvsService_stub* service, const TQString &fileName, 
                      const TQString &revA, const TQString &revB);

protected:
    virtual void keyPressEvent(TQKeyEvent *e);

private slots:
    void toggleSynchronize(bool b);
    void comboActivated(int index);
    void backClicked();
    void forwClicked();
    void saveAsClicked();

private:
    void newDiffHunk(int& linenoA, int& linenoB, const TQStringList& linesA,
                     const TQStringList& linesB);
    void callExternalDiff(const TQString& extdiff, CvsService_stub* service, 
                          const TQString& fileName, const TQString& revA, 
                          const TQString& revB);
    void updateNofN();
    void updateHighlight(int newitem);

    TQLabel *revlabel1, *revlabel2, *nofnlabel;
    TQCheckBox *syncbox;
    TQComboBox *itemscombo;
    TQPushButton *backbutton, *forwbutton;
    DiffView *diff1, *diff2;

    TQPtrList<DiffItem> items;
    int markeditem;
    KConfig& partConfig;
    TQStringList m_diffOutput;
};

#endif


// Local Variables:
// c-basic-offset: 4
// End:
