/*
 * Copyright (c) 2004-2007 André Wöbbeking <Woebbeking@kde.org>
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


#include "tooltip.h"

#include <kglobal.h>
#include <kglobalsettings.h>

#include <tqsimplerichtext.h>


namespace Cervisia
{


static TQString truncateLines(const TQString&, const TQFontMetrics&, const TQSize&);
static TQString truncateLines(const TQString&, const TQFont&, const TQPoint&, const TQRect&);


ToolTip::ToolTip(TQWidget* widget)
    : TQObject(widget), TQToolTip(widget)
{
}


void ToolTip::maybeTip(const TQPoint& pos)
{
    TQRect rect;
    TQString text;
    emit queryToolTip(pos, rect, text);

    if (rect.isValid() && !text.isEmpty())
    {
        text = truncateLines(text,
                             font(),
                             parentWidget()->mapToGlobal(pos),
                             KGlobalSettings::desktopGeometry(parentWidget()));
        tip(rect, text);
    }
}


// Primtive routine to truncate the text. size.width() is ignored, only
// size.height() is used at the moment to keep it fast. It doesn't work
// correct if text lines have different heights.
TQString truncateLines(const TQString&      text,
                      const TQFontMetrics& fm,
                      const TQSize&        size)
{
    const TQChar newLine('\n');

    const int lineSpacing(fm.lineSpacing());
    const int numberOfLines(text.contains(newLine) + 1);
    const int maxNumberOfLines(size.height() / lineSpacing);

    if (numberOfLines <= maxNumberOfLines)
        return text;

    const TQChar* tqunicode(text.tqunicode());
    for (int count(maxNumberOfLines); count; ++tqunicode)
        if (*tqunicode == newLine)
            --count;

    return text.left(tqunicode - text.tqunicode() - 1);
}


// Truncate the tooltip's text if necessary
TQString truncateLines(const TQString& text,
                      const TQFont&   font,
                      const TQPoint&  globalPos,
                      const TQRect&   desktopGeometry)
{
    // maximum size of the tooltip, - 10 just to be safe
    const int maxWidth(kMax(desktopGeometry.width() - globalPos.x(), globalPos.x())
                       - desktopGeometry.left() - 10);
    const int maxHeight(kMax(desktopGeometry.height() - globalPos.y(), globalPos.y())
                       - desktopGeometry.top() - 10);

    // calculate the tooltip's size
    const TQSimpleRichText layoutedText(text, font);

    // only if the tooltip's size is bigger in x- and y-direction the text must
    // be truncated otherwise the tip is moved to a position where it fits
    return  ((layoutedText.widthUsed() > maxWidth)
             && (layoutedText.height() > maxHeight))
        ? truncateLines(text, TQFontMetrics(font), TQSize(maxWidth, maxHeight))
        : text;
}


} // namespace Cervisia


#include "tooltip.moc"
