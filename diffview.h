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


#ifndef DIFFVIEW_H
#define DIFFVIEW_H


#include "qttableview.h"

#include <tqptrcollection.h>
#include <tqptrlist.h>


class KConfig;
class DiffViewItem;


class DiffViewItemList : public TQPtrList<DiffViewItem>
{
protected:
    virtual int compareItems(TQPtrCollection::Item item1, TQPtrCollection::Item item2);
};


class DiffView : public QtTableView
{
    Q_OBJECT

public:
    enum DiffType { Change, Insert, Delete, Neutral, Unchanged, Separator };

    DiffView( KConfig& cfg, bool withlinenos, bool withmarker,
              TQWidget *parent=0, const char *name=0 );

    void setPartner(DiffView *other);

    void up()
        { setTopCell(topCell()-1); }
    void down()
        { setTopCell(topCell()+1); }
    void next()
        { setTopCell(topCell()+viewHeight()/cellHeight()); }
    void prior()
        { setTopCell(topCell()-viewHeight()/cellHeight()); }

    void addLine(const TQString &line, DiffType type, int no=-1);
    TQString stringAtLine(int lineno);
    void setCenterLine(int lineno);
    void setInverted(int lineno, bool inverted);
    int count();
    void removeAtOffset(int offset);
    void insertAtOffset(const TQString &line, DiffType type, int offset);
    void setCenterOffset(int offset);
    TQString stringAtOffset(int offset);
    TQByteArray compressedContent();

    virtual void setFont(const TQFont &font);
    virtual int cellWidth(int col);
    virtual TQSize sizeHint() const;
    virtual void paintCell(TQPainter *p, int row, int col);
    virtual void wheelEvent(TQWheelEvent *);
    const TQScrollBar *scrollBar() const
        { return verticalScrollBar(); }

protected slots:
    void vertPositionChanged(int val);
    void horzPositionChanged(int val);

private:
    int findLine(int lineno);
    DiffViewItemList items;
    bool linenos;
    bool marker;
    int textwidth;
    DiffView *partner;
    static const int BORDER;

    TQColor diffChangeColor;
    TQColor diffInsertColor;
    TQColor diffDeleteColor;

    int m_tabWidth;
    KConfig& partConfig;
};


class DiffZoomWidget : public QFrame
{
    Q_OBJECT

public:
    DiffZoomWidget(KConfig& cfg, TQWidget *parent=0, const char *name=0);
    ~DiffZoomWidget();

    void setDiffView(DiffView *view);
    TQSize sizeHint() const;

protected:
    void paintEvent(TQPaintEvent *);
    bool eventFilter(TQObject *, TQEvent *e);

private:
    DiffView *diffview;

    TQColor diffChangeColor;
    TQColor diffInsertColor;
    TQColor diffDeleteColor;
};

#endif


// Local Variables:
// c-basic-offset: 4
// End:
