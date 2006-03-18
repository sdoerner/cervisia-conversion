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

#include "annotatedlg.h"

#include "annotateview.h"


AnnotateDialog::AnnotateDialog(KConfig& cfg, QWidget* parent, const char* name)
    : KDialogBase(parent, name, false, QString::null,
                  Close | Help, Close, true)
    , partConfig(cfg)
{
    m_view = new AnnotateView(this);
    setMainWidget(m_view);

    setHelp("annotate");

    setWFlags(Qt::WDestructiveClose | getWFlags());

    QSize size = configDialogSize(partConfig, "AnnotateDialog");
    resize(size);
}


AnnotateDialog::~AnnotateDialog()
{
    saveDialogSize(partConfig, "AnnotateDialog");
}


void AnnotateDialog::setAnnotateInfos(const Cervisia::LogInfoList& logInfos,
                                      const Cervisia::AnnotateInfoList& annotateInfos)
{
    using namespace Cervisia;

    setupLogInfoMap(logInfos);

    bool odd = false;
    QString oldRevision = "";

    AnnotateInfoList::ConstIterator it  = annotateInfos.begin();
    AnnotateInfoList::ConstIterator end = annotateInfos.end();
    for( ; it != end; ++it )
    {
        // retrieve log information for current revision
        Cervisia::LogInfo logInfo = m_logInfoMap[(*it).m_revision];

        // revision changed?
        if( logInfo.m_revision != oldRevision )
        {
            oldRevision = logInfo.m_revision;
            odd = !odd;
        }
        else
        {
            logInfo.m_author = QString::null;
            logInfo.m_revision = QString::null;
        }

        m_view->addLine(logInfo, (*it).m_line, odd);
    }
}


void AnnotateDialog::setupLogInfoMap(const Cervisia::LogInfoList& logInfos)
{
    using Cervisia::LogInfoList;

    LogInfoList::ConstIterator it  = logInfos.begin();
    LogInfoList::ConstIterator end = logInfos.end();
    for( ; it != end; ++it )
    {
        m_logInfoMap.insert((*it).m_revision, *it);
    }
}


// Local Variables:
// c-basic-offset: 4
// End:
