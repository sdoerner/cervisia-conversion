/*
 *  Copyright (C) 1999-2002 Bernd Gehrmann
 *                          bernd@mail.berlios.de
 *  Copyright (c) 2004 Christian Loose <christian.loose@hamburg.de>
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


#ifndef LOGTREE_H
#define LOGTREE_H


#include <tqptrlist.h>

#include <tqtable.h>


class LogTreeItem;
class LogTreeConnection;

namespace Cervisia
{
struct LogInfo;
}


typedef TQPtrList<LogTreeItem> LogTreeItemList;
typedef TQPtrList<LogTreeConnection> LogTreeConnectionList;


class LogTreeView : public TQTable
{
    Q_OBJECT
  TQ_OBJECT

public:
    explicit LogTreeView( TQWidget *tqparent=0, const char *name=0 );

    void addRevision(const Cervisia::LogInfo& logInfo);
    void setSelectedPair(TQString selectionA, TQString selectionB);
    void collectConnections();
    void recomputeCellSizes();
    virtual void paintCell(TQPainter *p, int row, int col, const TQRect& cr,
                           bool selected, const TQColorGroup& cg);

    virtual TQSize tqsizeHint() const;
    
    virtual TQString text(int row, int col) const;

signals:
    void revisionClicked(TQString rev, bool rmb);

protected:
    virtual void contentsMousePressEvent(TQMouseEvent *e);

private slots:

    void slotQueryToolTip(const TQPoint&, TQRect&, TQString&);

private:
    TQSize computeSize(const Cervisia::LogInfo&, int* = 0, int* = 0) const;
    void paintRevisionCell(TQPainter *p, int row, int col, const Cervisia::LogInfo& logInfo,
                           bool followed, bool branched, bool selected);
    void paintConnector(TQPainter *p, int row, int col, bool followed, bool branched);

    LogTreeItemList items;
    LogTreeConnectionList connections;
    int currentRow, currentCol;

    static const int BORDER;
    static const int INSPACE;
};

#endif


// Local Variables:
// c-basic-offset: 4
// End:
