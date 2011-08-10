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


#include "loglist.h"

#include <tqapplication.h>
#include <tqkeycode.h>
#include <klocale.h>

#include "loginfo.h"
#include "misc.h"
#include "tooltip.h"


class LogListViewItem : public KListViewItem
{
public:

    enum { Revision, Author, Date, Branch, Comment, Tags };

    LogListViewItem(TQListView* list, const Cervisia::LogInfo& logInfo);

    virtual int compare(TQListViewItem* i, int col, bool) const;

private:
    static TQString truncateLine(const TQString &s);

    Cervisia::LogInfo m_logInfo;
    friend class LogListView;
};


LogListViewItem::LogListViewItem(TQListView* list, const Cervisia::LogInfo& logInfo)
    : KListViewItem(list),
      m_logInfo(logInfo)
{
    setText(Revision, logInfo.m_revision);
    setText(Author, logInfo.m_author);
    setText(Date, logInfo.dateTimeToString());
    setText(Comment, truncateLine(logInfo.m_comment));

    for (Cervisia::LogInfo::TTagInfoSeq::const_iterator it = logInfo.m_tags.begin();
         it != logInfo.m_tags.end(); ++it)
    {
        const Cervisia::TagInfo& tagInfo(*it);

        if (tagInfo.m_type == Cervisia::TagInfo::OnBranch)
        {
            setText(Branch, tagInfo.m_name);
        }
    }

    setText(Tags, logInfo.tagsToString(Cervisia::TagInfo::Tag,
                                       Cervisia::LogInfo::NoTagType,
                                       TQString::tqfromLatin1(", ")));
}


TQString LogListViewItem::truncateLine(const TQString &s)
{
    int pos;

    TQString res = s.simplifyWhiteSpace();
    if ( (pos = res.find('\n')) != -1 )
        res = res.left(pos) + "...";

    return res;
}


int LogListViewItem::compare(TQListViewItem* i, int col, bool ascending) const
{
    const LogListViewItem* item = static_cast<LogListViewItem*>(i);

    int iResult;
    switch (col)
    {
    case Revision:
        iResult = ::compareRevisions(m_logInfo.m_revision, item->m_logInfo.m_revision);
        break;
    case Date:
        iResult = ::compare(m_logInfo.m_dateTime, item->m_logInfo.m_dateTime);
        break;
    default:
        iResult = TQListViewItem::compare(i, col, ascending);
    }

    return iResult;
}


LogListView::LogListView(KConfig& cfg, TQWidget *tqparent, const char *name)
    : KListView(tqparent, name)
    , partConfig(cfg)
{
    setAllColumnsShowFocus(true);
    setShowToolTips(false);
    setShowSortIndicator(true);
    setMultiSelection(true);
    setSorting(LogListViewItem::Revision, false);
    addColumn(i18n("Revision"));
    addColumn(i18n("Author"));
    addColumn(i18n("Date"));
    addColumn(i18n("Branch"));
    addColumn(i18n("Comment"));
    addColumn(i18n("Tags"));

    Cervisia::ToolTip* toolTip = new Cervisia::ToolTip(viewport());

    connect(toolTip, TQT_SIGNAL(queryToolTip(const TQPoint&, TQRect&, TQString&)),
            this, TQT_SLOT(slotQueryToolTip(const TQPoint&, TQRect&, TQString&)));

    // without this restoreLayout() can't change the column widths
    for (int i = 0; i < columns(); ++i)
        setColumnWidthMode(i, Manual);

    restoreLayout(&partConfig, TQString::tqfromLatin1("LogList view"));
}


LogListView::~LogListView()
{
    saveLayout(&partConfig, TQString::tqfromLatin1("LogList view"));
}


void LogListView::addRevision(const Cervisia::LogInfo& logInfo)
{
    (void) new LogListViewItem(this, logInfo);
}


void LogListView::setSelectedPair(const TQString &selectionA, const TQString &selectionB)
{
    for ( TQListViewItem *item = firstChild(); item;
	  item = item->nextSibling() )
	{
            LogListViewItem *i = static_cast<LogListViewItem*>(item);
            setSelected(i, (selectionA == i->text(LogListViewItem::Revision) ||
                            selectionB == i->text(LogListViewItem::Revision)) );
        }
}

void LogListView::contentsMousePressEvent(TQMouseEvent *e)
{
    // Retrieve selected item
    const LogListViewItem* selItem
        = static_cast<LogListViewItem*>(itemAt(contentsToViewport(e->pos())));
    if( !selItem )
        return;
        
    // Retrieve revision
    const TQString revision = selItem->text(LogListViewItem::Revision);   
    
    if ( e->button() == Qt::LeftButton )
    {
        // If the control key was pressed, then we change revision B not A
        if( e->state() & ControlButton )
            emit revisionClicked(revision, true);
        else
            emit revisionClicked(revision, false);
    }
    else if ( e->button() == Qt::MidButton )
        emit revisionClicked(revision, true);
}


void LogListView::keyPressEvent(TQKeyEvent *e)
{
    switch (e->key()) {
    case Key_A:
        if (currentItem())
            emit revisionClicked(currentItem()->text(LogListViewItem::Revision), false);
        break;
        break;
    case Key_B:
        if (currentItem())
            emit revisionClicked(currentItem()->text(LogListViewItem::Revision), true);
        break;
    case Key_Backspace:
    case Key_Delete:
    case Key_Down:
    case Key_Up:
    case Key_Home:
    case Key_End:
    case Key_Next:
    case Key_Prior:
        if (e->state() == 0)
             TQListView::keyPressEvent(e);
        else
            TQApplication::postEvent(this, new TQKeyEvent(TQEvent::KeyPress, e->key(), e->ascii(), 0));
        break;
    default:
        // Ignore Key_Enter, Key_Return
        e->ignore();
    }
}


void LogListView::slotQueryToolTip(const TQPoint& viewportPos,
                                   TQRect&        viewportRect,
                                   TQString&      text)
{
    if (const LogListViewItem* item = static_cast<LogListViewItem*>(itemAt(viewportPos)))
    {
        viewportRect = tqitemRect(item);
        text = item->m_logInfo.createToolTipText();
    }
}


#include "loglist.moc"

// Local Variables:
// c-basic-offset: 4
// End:
