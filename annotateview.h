/*
 * Copyright (C) 1999-2002 Bernd Gehrmann <bernd@mail.berlios.de>
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


#ifndef ANNOTATEVIEW_H
#define ANNOTATEVIEW_H


#include <tqlistview.h>


class KConfig;


namespace Cervisia
{
struct LogInfo;
}


class AnnotateView : public TQListView
{
    Q_OBJECT
  TQ_OBJECT

public:

    explicit AnnotateView( KConfig &cfg, TQWidget *parent=0, const char *name=0 );

    void addLine(const Cervisia::LogInfo& logInfo, const TQString& content,
                 bool odd);

    virtual TQSize tqsizeHint() const;

private slots:

    void slotQueryToolTip(const TQPoint&, TQRect&, TQString&);
};


#endif
