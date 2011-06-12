/*
 * Copyright (c) 2003-2007 André Wöbbeking <Woebbeking@kde.org>
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


#include "loginfo.h"

#include <tqstylesheet.h>

#include <kglobal.h>
#include <klocale.h>


namespace Cervisia
{


TagInfo::TagInfo(const TQString& name, Type type)
    : m_name(name),
      m_type(type)
{
}


TQString TagInfo::toString(bool prefixWithType) const
{
    TQString text;
    if (prefixWithType)
    {
        text += typeToString() + TQString::tqfromLatin1(": ");
    }
    text += m_name;

    return text;
}


TQString TagInfo::typeToString() const
{
    TQString text;
    switch (m_type)
    {
    case Branch:
        text = i18n("Branchpoint");
        break;
    case OnBranch:
        text = i18n("On Branch");
        break;
    case Tag:
        text = i18n("Tag");
        break;
    }

    return text;
}


TQString LogInfo::createToolTipText(bool showTime) const
{
    TQString text(TQString::tqfromLatin1("<nobr><b>"));
    text += TQStyleSheet::escape(m_revision);
    text += TQString::tqfromLatin1("</b>&nbsp;&nbsp;");
    text += TQStyleSheet::escape(m_author);
    text += TQString::tqfromLatin1("&nbsp;&nbsp;<b>");
    text += TQStyleSheet::escape(dateTimeToString(showTime));
    text += TQString::tqfromLatin1("</b></nobr>");

    if (!m_comment.isEmpty())
    {
        text += TQString::tqfromLatin1("<pre>");
        text += TQStyleSheet::escape(m_comment);
        text += TQString::tqfromLatin1("</pre>");
    }

    if (!m_tags.isEmpty())
    {
        text += TQString::tqfromLatin1("<i>");
        for (TTagInfoSeq::const_iterator it = m_tags.begin();
             it != m_tags.end(); ++it)
        {
            if (it != m_tags.begin() || m_comment.isEmpty())
                text += TQString::tqfromLatin1("<br>");
            text += TQStyleSheet::escape((*it).toString());
        }
        text += TQString::tqfromLatin1("</i>");
    }

    return text;
}


TQString LogInfo::dateTimeToString(bool showTime, bool shortFormat) const
{
    if( showTime )
        return KGlobal::locale()->formatDateTime(m_dateTime, shortFormat);
    else
        return KGlobal::locale()->formatDate(m_dateTime.date(), shortFormat);
}


TQString LogInfo::tagsToString(unsigned int types,
                              unsigned int prefixWithType,
                              const TQString& separator) const
{
    TQString text;
    for (TTagInfoSeq::const_iterator it = m_tags.begin();
         it != m_tags.end(); ++it)
    {
        const TagInfo& tagInfo(*it);

        if (tagInfo.m_type & types)
        {
            if (!text.isEmpty())
            {
                text += separator;
            }

            text += tagInfo.toString(tagInfo.m_type & prefixWithType);
        }
    }

    return text;
}


} // namespace Cervisia
