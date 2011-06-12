/*
 *  Copyright (c) 2003 Christian Loose <christian.loose@hamburg.de>
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


#include "logplainview.h"

#include <tqregexp.h>
#include <tqstringlist.h>
#include <tqstylesheet.h>
#include <kfind.h>
#include <kfinddialog.h>
#include <klocale.h>

#include "loginfo.h"

using namespace Cervisia;


LogPlainView::LogPlainView(TQWidget* tqparent, const char* name)
    : KTextBrowser(tqparent, name)
    , m_tqfind(0)
    , m_findPos(0)
{
    setNotifyClick(false);
}


LogPlainView::~LogPlainView()
{
    delete m_tqfind; m_tqfind = 0;
}


void LogPlainView::addRevision(const LogInfo& logInfo)
{
    setTextFormat(TQStyleSheet::RichText);

    // assemble revision information lines
    TQString logEntry;

    logEntry += "<b>" + i18n("revision %1").tqarg(TQStyleSheet::escape(logInfo.m_revision)) +
                "</b>";
    logEntry += " &nbsp;[<a href=\"revA#" + TQStyleSheet::escape(logInfo.m_revision) + "\">" +
                i18n("Select for revision A") +
                "</a>]";
    logEntry += " [<a href=\"revB#" + TQStyleSheet::escape(logInfo.m_revision) + "\">" +
                i18n("Select for revision B") +
                "</a>]<br>";
    logEntry += "<i>" +
                i18n("date: %1; author: %2").tqarg(TQStyleSheet::escape(logInfo.dateTimeToString()))
                                            .tqarg(TQStyleSheet::escape(logInfo.m_author)) +
                "</i>";

    append(logEntry);

    setTextFormat(TQStyleSheet::PlainText);

    const TQChar newline('\n');

    // split comment in separate lines
    TQStringList lines = TQStringList::split(newline, logInfo.m_comment, true);

    append(newline);
    TQStringList::Iterator it  = lines.begin();
    TQStringList::Iterator end = lines.end();
    for( ; it != end; ++it )
    {
        append((*it).isEmpty() ? TQString(newline) : *it);
    }
    append(newline);

    setTextFormat(TQStyleSheet::RichText);

    for( LogInfo::TTagInfoSeq::const_iterator it = logInfo.m_tags.begin();
         it != logInfo.m_tags.end(); ++it )
    {
        append("<i>" + TQStyleSheet::escape((*it).toString()) + "</i>");
    }

    // add an empty line when we had tags or branches
    if( !logInfo.m_tags.empty() )
    {
        setTextFormat(TQStyleSheet::PlainText);
        append(newline);
    }

    // add horizontal line
    setTextFormat(TQStyleSheet::RichText);
    append("<hr>");
}


void LogPlainView::searchText(int options, const TQString& pattern)
{
    m_tqfind = new KFind(pattern, options, this);

    connect(m_tqfind, TQT_SIGNAL(highlight(const TQString&, int, int)),
            this, TQT_SLOT(searchHighlight(const TQString&, int, int)));
    connect(m_tqfind, TQT_SIGNAL(findNext()),
           this, TQT_SLOT(findNext()));

    m_findPos = 0;
    if( options & KFindDialog::FromCursor )
    {
        const TQPoint pos(contentsX(), contentsY());
        m_findPos = paragraphAt(pos);
    }

    findNext();
}


void LogPlainView::scrollToTop()
{
    setContentsPos(0, 0);
}


void LogPlainView::findNext()
{
    static const TQRegExp breakLineTag("<br[^>]*>");
    static const TQRegExp htmlTags("<[^>]*>");

    KFind::Result res = KFind::NoMatch;

    while( res == KFind::NoMatch && m_findPos < paragraphs() && m_findPos >= 0 )
    {
        if( m_tqfind->needData() )
        {
            TQString richText = text(m_findPos);

            // tqreplace <br/> with '\n'
            richText.tqreplace(breakLineTag, "\n");

            // remove html tags from text
            richText.tqreplace(htmlTags, "");

            m_tqfind->setData(richText);
        }

        res = m_tqfind->find();

        if( res == KFind::NoMatch )
        {
            if( m_tqfind->options() & KFindDialog::FindBackwards )
                --m_findPos;
            else
                ++m_findPos;
        }
    }

    // reached the end?
    if( res == KFind::NoMatch )
    {
        if( m_tqfind->shouldRestart() )
        {
            m_findPos = 0;
            findNext();
        }
        else
        {
            delete m_tqfind;
            m_tqfind = 0;
        }
    }
}


void LogPlainView::searchHighlight(const TQString& text, int index, int length)
{
    Q_UNUSED(text);
    setSelection(m_findPos, index, m_findPos, index + length);
}


void LogPlainView::setSource(const TQString& name)
{
    if( name.isEmpty() )
        return;

    bool selectedRevisionB = name.startsWith("revB#");
    if( selectedRevisionB || name.startsWith("revA#") )
    {
        emit revisionClicked(name.mid(5), selectedRevisionB);
    }
}

#include "logplainview.moc"
