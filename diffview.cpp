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


#include "diffview.h"

#include <tqpainter.h>
#include <tqscrollbar.h>
#include <tqpixmap.h>
#include <tqregexp.h>
#include <tqstyle.h>

#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobalsettings.h>
#include <klocale.h>


class DiffViewItem
{
public:
    TQString line;
    DiffView::DiffType type;
    bool inverted;
    int no;
};


int DiffViewItemList::compareItems(TQPtrCollection::Item item1, TQPtrCollection::Item item2)
{
    return (static_cast<DiffViewItem*>(item1)->no
            == static_cast<DiffViewItem*>(item2)->no)? 0 : 1;
}


const int DiffView::BORDER = 7;


DiffView::DiffView( KConfig& cfg, bool withlinenos, bool withmarker,
                    TQWidget *tqparent, const char *name )
    : TQtTableView(tqparent, name, WRepaintNoErase)
    , partConfig(cfg)
{
    setNumRows(0);
    setNumCols( 1 + (withlinenos?1:0) + (withmarker?1:0) );
    setTableFlags( Tbl_autoVScrollBar|Tbl_autoHScrollBar|
                   Tbl_smoothVScrolling );
    setFrameStyle( TQFrame::WinPanel | TQFrame::Sunken );
    setBackgroundMode( PaletteBase );
    setWFlags( WResizeNoErase );

    partConfig.setGroup("LookAndFeel");
    setFont(partConfig.readFontEntry("DiffFont"));
    TQFontMetrics fm(font());
    setCellHeight(fm.lineSpacing());
    setCellWidth(0);
    textwidth = 0;

    partConfig.setGroup("General");
    m_tabWidth = partConfig.readNumEntry("TabWidth", 8);

    items.setAutoDelete(true);
    linenos = withlinenos;
    marker = withmarker;

    partConfig.setGroup("Colors");
    TQColor defaultColor=TQColor(237, 190, 190);
    diffChangeColor=partConfig.readColorEntry("DiffChange",&defaultColor);
    defaultColor=TQColor(190, 190, 237);
    diffInsertColor=partConfig.readColorEntry("DiffInsert",&defaultColor);
    defaultColor=TQColor(190, 237, 190);
    diffDeleteColor=partConfig.readColorEntry("DiffDelete",&defaultColor);
}


void DiffView::setFont(const TQFont &font)
{
    TQtTableView::setFont(font);
    TQFontMetrics fm(font);
    setCellHeight(fm.lineSpacing());
}


void DiffView::setPartner(DiffView *other)
{
    partner = other;
    if (partner)
    {
        connect( verticalScrollBar(), TQT_SIGNAL(valueChanged(int)),
                 TQT_SLOT(vertPositionChanged(int)) );
        connect( verticalScrollBar(), TQT_SIGNAL(sliderMoved(int)),
                 TQT_SLOT(vertPositionChanged(int)) );
        connect( horizontalScrollBar(), TQT_SIGNAL(valueChanged(int)),
                 TQT_SLOT(horzPositionChanged(int)) );
        connect( horizontalScrollBar(), TQT_SIGNAL(sliderMoved(int)),
                 TQT_SLOT(horzPositionChanged(int)) );
    }
}


void DiffView::vertPositionChanged(int val)
{
    if (partner)
        partner->setYOffset(TQMIN(val,partner->maxYOffset()));
}


void DiffView::horzPositionChanged(int val)
{
    if (partner)
        partner->setXOffset(TQMIN(val,partner->maxXOffset()));
}


// *offset methods are only for views withlineno
void DiffView::removeAtOffset(int offset)
{
    items.remove(offset);
    setNumRows(numRows()-1);
}


void DiffView::insertAtOffset(const TQString &line, DiffType type, int offset)
{
    DiffViewItem *item = new DiffViewItem;
    item->line = line;
    item->type = type;
    item->no = -1;
    item->inverted = false;
    items.insert(offset, item);
    setNumRows(numRows()+1);
}


void DiffView::setCenterOffset(int offset)
{
    if (!rowIsVisible(offset))
    {
        int visiblerows = viewHeight()/cellHeight(0);
        setTopCell( TQMAX(0, offset - visiblerows/2) );
    }
}


void DiffView::addLine(const TQString &line, DiffType type, int no)
{
    TQFont f(font());
    f.setBold(true);
    TQFontMetrics fmbold(f);
    TQFontMetrics fm(font());


    // calculate textwidth based on 'line' where tabs are expanded
    //
    // *Please note*
    // For some fonts, e.g. "Clean", is fm.maxWidth() greater than
    // fmbold.maxWidth().
    TQString copy(line);
    const int numTabs = copy.tqcontains('\t', false);
    copy.tqreplace( TQRegExp("\t"), "");

    const int tabSize   = m_tabWidth * TQMAX(fm.maxWidth(), fmbold.maxWidth());
    const int copyWidth = TQMAX(fm.width(copy), fmbold.width(copy));
    textwidth = TQMAX(textwidth, copyWidth + numTabs * tabSize);

    DiffViewItem *item = new DiffViewItem;
    item->line = line;
    item->type = type;
    item->no = no;
    item->inverted = false;
    items.append(item);
    setNumRows(numRows()+1);
}


TQString DiffView::stringAtOffset(int offset)
{
    if (offset >= (int)items.count())
    {
        kdDebug(8050) << "Internal error: lineAtOffset" << endl;
    }
    return items.at(offset)->line;
}


int DiffView::count()
{
    return items.count();
}


int DiffView::findLine(int lineno)
{
    int offset;
    DiffViewItem tmp;
    tmp.no = lineno;
    if ( (offset = items.tqfind(&tmp)) == -1)
    {
        kdDebug(8050) << "Internal Error: Line " << lineno << " not found" << endl;
        return -1;
    }
    return offset;
}


void DiffView::setInverted(int lineno, bool inverted)
{
    int offset;
    if ( (offset = findLine(lineno)) != -1)
        items.at(offset)->inverted = inverted;
}


void DiffView::setCenterLine(int lineno)
{
    int offset;
    if ( (offset = findLine(lineno)) != -1)
        setCenterOffset(offset);
}


TQString DiffView::stringAtLine(int lineno)
{
    int pos;
    if ( (pos = findLine(lineno)) != -1 )
        return items.at(pos)->line;
    else
        return TQString();
}


TQByteArray DiffView::compressedContent()
{
    TQByteArray res(items.count());

    TQPtrListIterator<DiffViewItem> it(items);
    int i=0;
    for (; it.current(); ++it)
    {
        switch (it.current()->type)
        {
            case Change:   res[i] = 'C'; break;
            case Insert:   res[i] = 'I'; break;
            case Delete:   res[i] = 'D'; break;
            case Neutral:  res[i] = 'N'; break;
            case Unchanged:res[i] = 'U'; break;
            default:       res[i] = ' ';
        }
        ++i;
    }
    return res;
}


int DiffView::cellWidth(int col)
{
    if (col == 0 && linenos)
    {
        TQFontMetrics fm(font());
        return fm.width("10000");
    }
    else if (marker && (col == 0 || col == 1))
    {
        TQFontMetrics fm( fontMetrics() );
        return TQMAX(TQMAX( fm.width(i18n("Delete")),
                          fm.width(i18n("Insert"))),
                    fm.width(i18n("Change")))+2*BORDER;
    }
    else
    {
        int rest = (linenos || marker)? cellWidth(0) : 0;
        if (linenos && marker)
            rest += cellWidth(1);
        return TQMAX(textwidth, viewWidth()-rest);
    }
}


TQSize DiffView::tqsizeHint() const
{
    TQFontMetrics fm(font());
    return TQSize( 4*fm.width("0123456789"), fm.lineSpacing()*8 );
}


void DiffView::paintCell(TQPainter *p, int row, int col)
{
    TQFontMetrics fm(font());
    p->setTabStops(m_tabWidth * fm.maxWidth());

    DiffViewItem *item = items.at(row);

    int width = cellWidth(col);
    int height = cellHeight();

    TQColor backgroundColor;
    bool inverted;
    int align;
    int innerborder;
    TQString str;

    TQFont oldFont(p->font());
    if (item->type==Separator)
    {
        backgroundColor = KGlobalSettings::highlightColor();
        p->setPen(KGlobalSettings::highlightedTextColor());
        inverted = false;
        align = AlignLeft;
        innerborder = 0;
        if (col == (linenos?1:0) + (marker?1:0))
            str = item->line;
        TQFont f(oldFont);
        f.setBold(true);
        p->setFont(f);
    }
    else if (col == 0 && linenos)
    {
        backgroundColor = KGlobalSettings::highlightColor();
        p->setPen(KGlobalSettings::highlightedTextColor());
        inverted = false;
        align = AlignLeft;
        innerborder = 0;
        if (item->no == -1)
            str = "+++++";
        else
            str.setNum(item->no);
    }
    else if (marker && (col == 0 || col == 1))
    {
        backgroundColor = KGlobalSettings::alternateBackgroundColor();
        p->setPen(KGlobalSettings::textColor());
        inverted = false;
        align = AlignRight;
        innerborder = BORDER;
        str = (item->type==Change)? i18n("Change")
            : (item->type==Insert)? i18n("Insert")
            : (item->type==Delete)? i18n("Delete") : TQString();
    }
    else
    {
        backgroundColor =
            (item->type==Change)? diffChangeColor
            : (item->type==Insert)? diffInsertColor
            : (item->type==Delete)? diffDeleteColor
            : (item->type==Neutral)? KGlobalSettings::alternateBackgroundColor() : KGlobalSettings::baseColor();
        p->setPen(KGlobalSettings::textColor());
        inverted = item->inverted;
        align = AlignLeft;
        innerborder = 0;
        str = item->line;
    }

    if (inverted)
    {
        p->setPen(backgroundColor);
        backgroundColor = KGlobalSettings::textColor();
        TQFont f(oldFont);
        f.setBold(true);
        p->setFont(f);
    }

    p->fillRect(0, 0, width, height, backgroundColor);
    p->drawText(innerborder, 0, width-2*innerborder, height, align|ExpandTabs, str);
    p->setFont(oldFont);
}


void DiffView::wheelEvent(TQWheelEvent *e)
{
    TQApplication::sendEvent(verticalScrollBar(), e);
}


DiffZoomWidget::DiffZoomWidget(KConfig& cfg, TQWidget *tqparent, const char *name)
    : TQFrame(tqparent, name)
{
    tqsetSizePolicy( TQSizePolicy( TQSizePolicy::Fixed, TQSizePolicy::Minimum ) );

    cfg.setGroup("Colors");
    TQColor defaultColor=TQColor(237, 190, 190);
    diffChangeColor=cfg.readColorEntry("DiffChange",&defaultColor);
    defaultColor=TQColor(190, 190, 237);
    diffInsertColor=cfg.readColorEntry("DiffInsert",&defaultColor);
    defaultColor=TQColor(190, 237, 190);
    diffDeleteColor=cfg.readColorEntry("DiffDelete",&defaultColor);
}


DiffZoomWidget::~DiffZoomWidget()
{}


void DiffZoomWidget::setDiffView(DiffView *view)
{
    diffview = view;
    TQScrollBar *sb = const_cast<TQScrollBar*>(diffview->scrollBar());
    sb->installEventFilter(this);
}


TQSize DiffZoomWidget::tqsizeHint() const
{
    return TQSize(25, tqstyle().tqpixelMetric(TQStyle::PM_ScrollBarExtent, this));
}


bool DiffZoomWidget::eventFilter(TQObject *o, TQEvent *e)
{
    if (e->type() == TQEvent::Show
        || e->type() == TQEvent::Hide
        || e->type() == TQEvent::Resize)
        tqrepaint();

    return TQFrame::eventFilter(o, e);
}


void DiffZoomWidget::paintEvent(TQPaintEvent *)
{
    const TQScrollBar* scrollBar = diffview->scrollBar();
    if (!scrollBar)
        return;

    // only y and height are important
    const TQRect scrollBarGroove(scrollBar->isVisible()
                                ? tqstyle().querySubControlMetrics(TQStyle::CC_ScrollBar,
                                                                 scrollBar,
                                                                 TQStyle::SC_ScrollBarGroove)
                                : rect());

    // draw rectangles at the positions of the differences

    const TQByteArray& lineTypes(diffview->compressedContent());

    TQPixmap pixbuf(width(), scrollBarGroove.height());
    pixbuf.fill(KGlobalSettings::baseColor());

    TQPainter p(&pixbuf, this);
    if (const unsigned int numberOfLines = lineTypes.size())
    {
        const double scale(((double) scrollBarGroove.height()) / numberOfLines);
        for (unsigned int index(0); index < numberOfLines;)
        {
            const char lineType(lineTypes[index]);

            // don't use tqRound() to avoid painting outside of the pixmap
            // (yPos1 must be lesser than scrollBarGroove.height())
            const int yPos1(static_cast<int>(index * scale));

            // search next line with different lineType
            for (++index; index < numberOfLines && lineType == lineTypes[index]; ++index)
                ;

            TQColor color;
            switch (lineType)
            {
            case 'C':
                color = diffChangeColor;
                break;
            case 'I':
                color = diffInsertColor;
                break;
            case 'D':
                color = diffDeleteColor;
                break;
            case ' ':
            case 'N':
                color = KGlobalSettings::alternateBackgroundColor();
                break;
            }

            if (color.isValid())
            {
                const int yPos2(tqRound(index * scale));
                const int areaHeight((yPos2 != yPos1) ? yPos2 - yPos1 : 1);

                p.fillRect(0, yPos1, pixbuf.width(), areaHeight, TQBrush(color));
            }
        }
    }
    p.flush();
    bitBlt(this, 0, scrollBarGroove.y(), &pixbuf);
}

#include "diffview.moc"


// Local Variables:
// c-basic-offset: 4
// End:
