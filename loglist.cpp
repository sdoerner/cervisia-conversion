/* 
 *  Copyright (C) 1999-2002 Bernd Gehrmann
 *                          bernd@mail.berlios.de
 *
 * This program may be distributed under the terms of the Q Public
 * License as defined by Trolltech AS of Norway and appearing in the
 * file LICENSE.QPL included in the packaging of this file.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */


#include "loglist.h"

#include <qheader.h>
#include <qkeycode.h>
#include <qstrlist.h>
#include <qstylesheet.h>
#include <kapplication.h>
#include <kconfig.h>
#include <klocale.h>

#include "tiplabel.h"
#include "misc.h"


class LogListViewItem : public QListViewItem
{
public:

    enum { Revision, Author, Date, Branch, Comment, Tags };

    LogListViewItem(QListView *list,
                    const QString &rev, const QString &author, const QString &date,
                    const QString &comment, const QString &tagcomment);

    virtual int compare(QListViewItem* i, int col, bool) const;

private:
    static QString truncateLine(const QString &s);
    static QString extractOrdinaryTags(const QString &s);
    static QString extractBranchName(const QString &s);

    QString mrev, mauthor, mdate;
    QString mcomment, mtagcomment;
    friend class LogListView;
};


LogListViewItem::LogListViewItem( QListView *list,
                                  const QString &rev, const QString &author, const QString &date,
                                  const QString &comment, const QString &tagcomment )
    : QListViewItem(list, rev, author, date+" ",
                    extractBranchName(tagcomment), truncateLine(comment), extractOrdinaryTags(tagcomment)),
    mrev(rev), mauthor(author), mdate(date), mcomment(comment), mtagcomment(tagcomment)
{
}


QString LogListViewItem::truncateLine(const QString &s)
{
    int pos;

    QString res = s.simplifyWhiteSpace();
    if ( (pos = res.find('\n')) != -1 )
        res = res.left(pos) + "...";

    return res;
}


static void takeLine(QString *s, QString *line)
{
    int pos = s->find('\n');
    if (pos == -1)
        {
            *line = *s;
            s->remove(0, s->length());
        }
    else
        {
            *line = s->left(pos);
            s->remove(0, pos+1);
        }
}


QString LogListViewItem::extractOrdinaryTags(const QString &s)
{
    QString res;

    // Note: same translation as in logdlg.cpp
    // This is a hack...
    QString prefix = i18n("\nTag: ");
    prefix.remove(0, 1);

    QString rest = s;
    while ( !rest.isEmpty() )
        {
            QString line;
            takeLine(&rest, &line);
            if (line.left(prefix.length()) == prefix)
                {
                    res += ", ";
                    res += line.right(line.length()-prefix.length());
                }
        }
    if (!res.isEmpty())
        res.remove(0, 2);
    return res;
}


QString LogListViewItem::extractBranchName(const QString &s)
{
    // Note: same translation as in logdlg.cpp
    // This is a hack...
    QString prefix = i18n("\nOn branch: ");
    prefix.remove(0, 1);

    QString rest = s;
    while ( !rest.isEmpty() )
        {
            QString line;
            takeLine(&rest, &line);
            if (line.left(prefix.length()) == prefix)
                return line.right(line.length()-prefix.length());
        }
    
    return "";
}


int LogListViewItem::compare(QListViewItem* i, int col, bool ascending) const
{
    LogListViewItem const* pItem = static_cast<LogListViewItem*>(i);

    int iResult;
    switch (col)
    {
    case Revision:
        iResult = ::compareRevisions(mrev, pItem->mrev);
        break;
    default:
        iResult = QListViewItem::compare(i, col, ascending);
    }

    return iResult;
}


LogListView::Options *LogListView::options = 0;
#define COLS 6


LogListView::LogListView(QWidget *parent, const char *name)
    : ListView(parent, name)
{
    setAllColumnsShowFocus(true);
    setShowToolTips(false);
    setShowSortIndicator(true);
    setMultiSelection(true);
    setSorting(LogListViewItem::Revision, false);
    setSorting(LogListViewItem::Branch, false);
    addColumn(i18n("Revision"));
    addColumn(i18n("Author"));
    addColumn(i18n("Date"));
    addColumn(i18n("Branch"));
    addColumn(i18n("Comment"));
    addColumn(i18n("Tags"));

    connect( this, SIGNAL(contentsMoving(int, int)), this, SLOT(hideLabel()) );

    currentTipItem = 0;
    currentLabel = 0;
    
    if (options)
        {
            for (int i=0; i<header()->count(); ++i)
                setColumnWidthMode(i, Manual);
            setColumnConfig(options->sortColumn, options->sortAscending,
                            options->indexToColumn, options->columnSizes);
        }
}


LogListView::~LogListView()
{
    delete currentLabel;

    if (!options)
        options = new Options;
    getColumnConfig(&options->sortColumn, &options->sortAscending,
                    &options->indexToColumn, &options->columnSizes);
}


void LogListView::loadOptions(KConfig *config)
{
    if (!config->readEntry("Customized"))
        return;

    options = new Options;
    options->sortColumn = config->readNumEntry("SortColumn");
    options->sortAscending = config->readBoolEntry("SortAscending");

    QValueList<int> list; QValueList<int>::Iterator it;
    int i, n;

    list = config->readIntListEntry("Columns");
    n = list.count();
    options->indexToColumn.resize(n);
    for (it = list.begin(), i=0;
         it != list.end() && i<n;
         ++it, ++i)
        options->indexToColumn.at(i) = (*it);
    
    list = config->readIntListEntry("ColumnSizes");
    n = list.count();
    options->columnSizes.resize(n);
    for (it = list.begin(), i=0;
         it != list.end() && i<n;
         ++it, ++i)
        options->columnSizes.at(i) = (*it);
}


void LogListView::saveOptions(KConfig *config)
{
    if (!options)
        return;

    config->writeEntry("Customized", true);
    config->writeEntry("SortColumn", options->sortColumn);
    config->writeEntry("SortAscending", options->sortAscending);
    QStringList indexList;
    for (uint i=0; i<options->indexToColumn.count(); ++i)
        indexList << QString::number(options->indexToColumn.at(i));
    config->writeEntry("Columns", indexList);
    QStringList sizeList;
    for (uint i=0; i<options->columnSizes.count(); ++i)
        sizeList << QString::number(options->columnSizes.at(i));
    config->writeEntry("ColumnSizes", sizeList);
}


void LogListView::hideLabel()
{
    delete currentLabel;
    currentLabel = 0;
}


void LogListView::addRevision(const QString &rev, const QString &author, const QString &date,
                              const QString &comment, const QString &tagcomment)
{
    (void) new LogListViewItem(this, rev, author, date, comment, tagcomment);
}


void LogListView::setSelectedPair(const QString &selectionA, const QString &selectionB)
{
    for ( QListViewItem *item = firstChild(); item;
	  item = item->nextSibling() )
	{
            LogListViewItem *i = static_cast<LogListViewItem*>(item);
            setSelected(i, (selectionA == i->text(0) ||
                            selectionB == i->text(0)) );
        }
}

void LogListView::contentsMousePressEvent(QMouseEvent *e)
{
    if ( e->button() == LeftButton )
	{
            QPoint vp = contentsToViewport(e->pos());
	    LogListViewItem *selItem
                = static_cast<LogListViewItem*>( itemAt(vp) );
            if (selItem)
                emit revisionClicked(selItem->text(LogListViewItem::Revision), false);
        }
    else if ( e->button() == MidButton )
        {
            QPoint vp = contentsToViewport(e->pos());
            LogListViewItem *selItem
                = static_cast<LogListViewItem*>( itemAt(vp) );
            if (selItem)
                emit revisionClicked(selItem->text(LogListViewItem::Revision), true);
	}
}


void LogListView::contentsMouseMoveEvent(QMouseEvent *e)
{
    if (!isActiveWindow())
        return;

    QPoint vp = contentsToViewport(e->pos());
    LogListViewItem *item
        = static_cast<LogListViewItem*>( itemAt(vp) );

    if (item != currentTipItem)
        hideLabel();

    if (!currentLabel && item)
        {
            QString text = "<qt><b>";
            text += QStyleSheet::escape(item->mrev);
            text += "</b>&nbsp;&nbsp;";
            text += QStyleSheet::escape(item->mauthor);
            text += "&nbsp;&nbsp;<b>";
            text += QStyleSheet::escape(item->mdate);
            text += "</b>";
            QStringList list2 = QStringList::split("\n", item->mcomment);
            QStringList::Iterator it2;
            for (it2 = list2.begin(); it2 != list2.end(); ++it2)
                {
                    text += "<br>";
                    text += QStyleSheet::escape(*it2);
                }
            if (!item->mtagcomment.isEmpty())
                {
                    text += "<i>";
                    QStringList list3 = QStringList::split("\n", item->mtagcomment);
                    QStringList::Iterator it3;
                    for (it3 = list3.begin(); it3 != list3.end(); ++it3)
                        {
                            text += "<br>";
                            text += QStyleSheet::escape(*it3);
                        }
                    text += "</i>";
                }
            text += "</qt>";

            int left = static_cast<QMouseEvent*>(e)->pos().x() + 20;
            int top = viewport()->mapTo(this, itemRect(item).bottomLeft()).y();
#if 0
            int vpx = contentsToViewport(static_cast<QMouseEvent*>(e)->pos()).x();
            int index = header()->mapToIndex(header()->sectionAt(vpx));
            if (index < columns()-1)
                left = header()->sectionPos(header()->mapToSection(index+1));
#endif
            currentLabel = new TipLabel(text);
            currentLabel->showAt(mapToGlobal(QPoint(left, top)));
            currentTipItem = item;
        }
}


void LogListView::windowActivationChange(bool oldActive)
{
    hideLabel();
    QListView::windowActivationChange(oldActive);
}


void LogListView::leaveEvent(QEvent *e)
{
    hideLabel();
    QListView::leaveEvent(e);
}


void LogListView::keyPressEvent(QKeyEvent *e)
{
    hideLabel();

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
             QListView::keyPressEvent(e);
        else
            kapp->postEvent(this, new QKeyEvent(QEvent::KeyPress, e->key(), e->ascii(), 0));
        break;
    default:
        // Ignore Key_Enter, Key_Return
        e->ignore();
    }
}

#include "loglist.moc"

// Local Variables:
// c-basic-offset: 4
// End:
