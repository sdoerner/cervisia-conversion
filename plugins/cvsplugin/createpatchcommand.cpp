/*
 * Copyright (c) 2005 Christian Loose <christian.loose@kdemail.net>
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

#include "createpatchcommand.h"
using Cervisia::CreatePatchCommand;

#include <qapplication.h>
#include <dcopref.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <cvsservice_stub.h>
#include <misc.h>
#include <patchoptiondlg.h>
#include <progressdlg.h>

#include "cvsplugin.h"


CreatePatchCommand::CreatePatchCommand(const QString& fileName,
                                       const QString& revisionA,
                                       const QString& revisionB)
    : CvsCommandBase(Other)
    , m_fileName(fileName)
    , m_revisionA(revisionA)
    , m_revisionB(revisionB)
{
    m_errorId1 = "cvs diff:";
    m_errorId2 = "cvs [diff aborted]:";
}


CreatePatchCommand::~CreatePatchCommand()
{
}


bool CreatePatchCommand::prepare()
{
    Cervisia::PatchOptionDialog optionDlg;
    if( optionDlg.exec() == KDialogBase::Rejected )
        return false;

    QStringList options;

    QString format = optionDlg.formatOption().stripWhiteSpace();
    if( !format.isEmpty() )
        options += format;

    QString diffOptions = optionDlg.diffOptions().stripWhiteSpace();
    if( !diffOptions.isEmpty() )
        options += QStringList::split(" ", diffOptions);

    DCOPRef jobRef = CvsPlugin::cvsService()->diff(m_fileName, m_revisionA,
                                                   m_revisionB, options);
    connectToJob(jobRef, ManualDeletion);

    connect(this, SIGNAL(receivedLine(const QString&)),
            this, SLOT(receivedLine(const QString&)));
    connect(this, SIGNAL(jobExited(bool, int)),
            this, SLOT(saveToFile()));

    return true;
}


void CreatePatchCommand::execute()
{
    ProgressDialog* dlg = new ProgressDialog(qApp->activeWindow(), i18n("Diffing"), i18n("CVS Diff"));
    dlg->execute(this);

    CvsCommandBase::execute();
}


void CreatePatchCommand::receivedLine(const QString& line)
{
    m_processOutput += line;
}


void CreatePatchCommand::saveToFile()
{
    // error occurred or process was canceled
    if( m_errorOccurred )
        return;

    QString patchFileName = KFileDialog::getSaveFileName();
    if( patchFileName.isEmpty() )
        return;

    if( !Cervisia::CheckOverwrite(patchFileName) )
        return;

    QFile f(patchFileName);
    if( !f.open(IO_WriteOnly) )
    {
        KMessageBox::sorry(qApp->activeWindow(),
                           i18n("Could not open file for writing."),
                           "Cervisia");
        return;
    }

    QTextStream t(&f);

    QStringList::ConstIterator it  = m_processOutput.begin();
    QStringList::ConstIterator end = m_processOutput.end();
    for( ; it != end; ++it )
        t << *it << '\n';

    f.close();

    deleteLater();
}

#include "createpatchcommand.moc"
