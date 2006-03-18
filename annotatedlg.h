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

#ifndef ANNOTATEDLG_H
#define ANNOTATEDLG_H

#include <kdialogbase.h>

#include <qmap.h>

#include "annotate_info.h"
#include "loginfo.h"

class AnnotateView;
class KConfig;


class AnnotateDialog : public KDialogBase
{
public:
    explicit AnnotateDialog(KConfig& cfg, QWidget* parent=0, const char* name=0);
    virtual ~AnnotateDialog();

    void setAnnotateInfos(const Cervisia::LogInfoList& logInfos,
                          const Cervisia::AnnotateInfoList& annotateInfos);

private:
    void setupLogInfoMap(const Cervisia::LogInfoList& logInfos);

    AnnotateView* m_view;
    KConfig&      partConfig;

    QMap<QString, Cervisia::LogInfo> m_logInfoMap;
};


#endif


// Local Variables:
// c-basic-offset: 4
// End:
