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


#ifndef LOGLIST_H
#define LOGLIST_H


#include <klistview.h>


class KConfig;

class TipLabel;
class LogListViewItem;

namespace Cervisia
{
struct LogInfo;
}


class LogListView : public KListView
{
    Q_OBJECT
  TQ_OBJECT
    
public:
    explicit LogListView( KConfig& cfg, TQWidget *tqparent=0, const char *name=0 );
    virtual ~LogListView();
    
    void addRevision(const Cervisia::LogInfo& logInfo);
    void setSelectedPair(const TQString &selectionA, const TQString &selectionB);

signals:
    void revisionClicked(TQString rev, bool rmb);

protected:
    virtual void contentsMousePressEvent(TQMouseEvent *e);
    virtual void keyPressEvent(TQKeyEvent *e);

private slots:

    void slotQueryToolTip(const TQPoint&, TQRect&, TQString&);

private:

    KConfig& partConfig;
};

#endif


// Local Variables:
// c-basic-offset: 4
// End:
