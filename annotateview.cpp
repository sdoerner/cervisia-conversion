/*
 * Copyright (C) 1999-2002 Bernd Gehrmann <bernd@mail.berlios.de>
 * Copyright (c) 2003-2005 Andr� W�bbeking <Woebbeking@web.de>
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

#include "annotateview.h"

#include <q3header.h>
#include <qpainter.h>
//Added by qt3to4:
#include <Q3Frame>
#include <kconfig.h>
#include <kglobalsettings.h>

#include "loginfo.h"
#include "tooltip.h"


using namespace Cervisia;


class AnnotateViewItem : public Q3ListViewItem
{
public:
    enum { LineNumberColumn, AuthorColumn, ContentColumn };

    AnnotateViewItem(AnnotateView *parent, const LogInfo& logInfo,
                     const QString &content, bool odd, int linenumber);

    virtual int compare(Q3ListViewItem *item, int col, bool ascending) const;
    virtual int width(const QFontMetrics &, const Q3ListView *, int col) const;
    virtual QString text(int col) const;
    virtual void paintCell(QPainter *, const QColorGroup &, int, int, int);

private:
    LogInfo m_logInfo;
    QString m_content;
    bool    m_odd;
    int     m_lineNumber;
    friend class AnnotateView;

    static const int BORDER;
};


const int AnnotateViewItem::BORDER = 4;


AnnotateViewItem::AnnotateViewItem(AnnotateView *parent, const LogInfo& logInfo,
                                   const QString &content, bool odd, int linenumber)
    : Q3ListViewItem(parent)
    , m_logInfo(logInfo)
    , m_content(content)
    , m_odd(odd)
    , m_lineNumber(linenumber)
{}


int AnnotateViewItem::compare(Q3ListViewItem *item, int, bool) const
{
    int linenum1 = m_lineNumber;
    int linenum2 = static_cast<AnnotateViewItem*>(item)->m_lineNumber;

    return (linenum2 > linenum1)? -1 : (linenum2 < linenum1)? 1 : 0;
}


QString AnnotateViewItem::text(int col) const
{
    switch (col)
    {
    case LineNumberColumn:
        return QString::number(m_lineNumber);
    case AuthorColumn:
        if( m_logInfo.m_author.isNull() )
            return QString::null;
        else
            return (m_logInfo.m_author + QChar(' ') + m_logInfo.m_revision);
    case ContentColumn:
        return m_content;
    default:
        ;
    };

    return QString::null;
}


void AnnotateViewItem::paintCell(QPainter *p, const QColorGroup &, int col, int width, int align)
{
    QColor backgroundColor;

    switch (col)
    {
    case LineNumberColumn:
        backgroundColor = KGlobalSettings::highlightColor();
        p->setPen(KGlobalSettings::highlightedTextColor());
        break;
    default:
        backgroundColor = m_odd ? KGlobalSettings::baseColor()
                                : KGlobalSettings::alternateBackgroundColor();
        p->setPen(KGlobalSettings::textColor());
        break;
    };

    p->fillRect(0, 0, width, height(), backgroundColor);

    QString str = text(col);
    if (str.isEmpty())
        return;

    if (align & (Qt::AlignTop || Qt::AlignBottom) == 0)
            align |= Qt::AlignVCenter;

    p->drawText(BORDER, 0, width - 2*BORDER, height(), align, str);
}



int AnnotateViewItem::width(const QFontMetrics &fm, const Q3ListView *, int col) const
{
    return fm.width(text(col)) + 2*BORDER;
}


/*!
  @todo The dummy column (remaining space eater) doesn't work
  caused by a bug in QHeader::adjustHeaderSize() in Qt <= 3.0.4.
*/

AnnotateView::AnnotateView(KConfig &cfg, QWidget *parent, const char *name)
    : Q3ListView(parent, name, Qt::WNoAutoErase | Qt::WResizeNoErase)
{
    setFrameStyle(Q3Frame::WinPanel | Q3Frame::Sunken);
    setAllColumnsShowFocus(true);
    setShowToolTips(false);
    setSelectionMode(NoSelection);
    header()->hide();
    //    setResizeMode(LastColumn);

    addColumn(QString::null);
    addColumn(QString::null);
    addColumn(QString::null);

    setSorting(AnnotateViewItem::LineNumberColumn);
    setColumnAlignment(AnnotateViewItem::LineNumberColumn, Qt::AlignRight);

    ToolTip* toolTip = new ToolTip(viewport());

    connect(toolTip, SIGNAL(queryToolTip(const QPoint&, QRect&, QString&)),
            this, SLOT(slotQueryToolTip(const QPoint&, QRect&, QString&)));

    KConfigGroupSaver cs(&cfg, "LookAndFeel");
    setFont(cfg.readFontEntry("AnnotateFont"));
}



void AnnotateView::addLine(const LogInfo& logInfo, const QString& content,
                           bool odd)
{
    new AnnotateViewItem(this, logInfo, content, odd, childCount()+1);
}


QSize AnnotateView::sizeHint() const
{
    QFontMetrics fm(fontMetrics());
    return QSize(100 * fm.width("0"), 10 * fm.lineSpacing());
}


void AnnotateView::slotQueryToolTip(const QPoint& viewportPos,
                                    QRect&        viewportRect,
                                    QString&      text)
{
    if (const AnnotateViewItem* item = static_cast<AnnotateViewItem*>(itemAt(viewportPos)))
    {
        const int column(header()->sectionAt(viewportPos.x()));
        if ((column == AnnotateViewItem::AuthorColumn) && !item->m_logInfo.m_author.isNull())
        {
            viewportRect = itemRect(item);
            text = item->m_logInfo.createToolTipText(false);
        }
    }
}


#include "annotateview.moc"


// Local Variables:
// c-basic-offset: 4
// End:
