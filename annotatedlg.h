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


#ifndef ANNOTATEDLG_H
#define ANNOTATEDLG_H


#include <kdialogbase.h>


class AnnotateView;
class QDate;
class KConfig;

namespace Cervisia
{
struct LogInfo;
}


class AnnotateDialog : public KDialogBase
{
public:

    explicit AnnotateDialog( KConfig& cfg, QWidget *parent=0, const char *name=0 );

    virtual ~AnnotateDialog();

    void addLine(const Cervisia::LogInfo& logInfo, const QString& content,
                 bool odd);

private:
    AnnotateView *annotate;
    KConfig&      partConfig;
};

#endif


// Local Variables:
// c-basic-offset: 4
// End:
