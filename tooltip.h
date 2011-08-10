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


#ifndef CERVISIA_TOOLTIP_H
#define CERVISIA_TOOLTIP_H


#include <tqobject.h>
#include <tqtooltip.h>


namespace Cervisia
{


/**
 * This class extends TQToolTip:
 * - no more need to subclass just connect to the signal queryToolTip()
 * - truncate too large tooltip texts.
 */
class ToolTip : public TQObject, public TQToolTip
{
    Q_OBJECT
  TQ_OBJECT

public:

    /**
     * @param widget The widget you want to add tooltips to. It's also used as
     * parent for the TQObject. So you don't have to free an instance of this
     * class yourself.
     */
    explicit ToolTip(TQWidget* widget);

signals:

    /**
     * This signal is emitted when a tooltip could be displayed. When a client
     * wants to display anythink it must set a valid tooltip rectangle and a
     * non empty text.
     *
     * @param pos The position of the tooltip in the parent widget's coordinate system.
     *
     * @param rect The rectangle in the parent widget's coordinate system where the
     * tooltip is valid.
     *
     * @param text The tooltip text.
     */
    void queryToolTip(const TQPoint& pos, TQRect& rect, TQString& text);

protected:

    virtual void maybeTip(const TQPoint&);
};


} // namespace Cervisia


#endif // CERVISIA_TOOLTIP_H
