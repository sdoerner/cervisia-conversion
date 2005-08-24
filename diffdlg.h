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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */


#ifndef DIFFDLG_H
#define DIFFDLG_H

#include <qptrlist.h>
#include <kdialogbase.h>

#include "diff_info.h"


class QLabel;
class QCheckBox;
class QComboBox;
class KConfig;
class DiffItem;
class DiffView;
class CvsService_stub;


class DiffDialog : public KDialogBase
{
    Q_OBJECT

public:

    explicit DiffDialog( KConfig& config, QWidget *parent=0, const char *name=0, 
                         bool modal=false );

    virtual ~DiffDialog();

    bool parseCvsDiff(CvsService_stub* service, const QString &fileName, 
                      const QString &revA, const QString &revB);

    void setDiffInfos(const Cervisia::DiffInfoList& diffInfos);

protected:
    virtual void keyPressEvent(QKeyEvent *e);

private slots:
    void toggleSynchronize(bool b);
    void comboActivated(int index);
    void backClicked();
    void forwClicked();
    void saveAsClicked();

private:
    void newDiffHunk(int& linenoA, int& linenoB, const QStringList& linesA,
                     const QStringList& linesB);
    void callExternalDiff(const QString& extdiff, CvsService_stub* service, 
                          const QString& fileName, const QString& revA, 
                          const QString& revB);
    void updateNofN();
    void updateHighlight(int newitem);

    QLabel *revlabel1, *revlabel2, *nofnlabel;
    QCheckBox *syncbox;
    QComboBox *itemscombo;
    QPushButton *backbutton, *forwbutton;
    DiffView *diff1, *diff2;

    QPtrList<DiffItem> items;
    int markeditem;
    KConfig& partConfig;
    Cervisia::DiffInfoList m_diffInfos;
};

#endif


// Local Variables:
// c-basic-offset: 4
// End:
