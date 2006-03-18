/*
 *  Copyright (C) 1999-2002 Bernd Gehrmann
 *                          bernd@physik.hu-berlin.de
 *  Copyright (c) 2003-2005 Christian Loose <christian.loose@kdemail.net>
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


#include "protocolview.h"

#include <qdir.h>
#include <qpopupmenu.h>
#include <dcopref.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "cervisiasettings.h"
#include "commandbase.h"
#include "cvsjob_stub.h"
#include "pluginbase.h"
#include "pluginmanager.h"


using namespace Cervisia;


ProtocolView::ProtocolView(QWidget *parent, const char *name)
    : QTextEdit(parent, name)
    , m_isUpdateJob(false)
    , m_currentCmd(0)
{
    setReadOnly(true);
    setUndoRedoEnabled(false);
    setTabChangesFocus(true);
    setTextFormat(Qt::LogText);

    setFont(CervisiaSettings::protocolFont());

    conflictColor = CervisiaSettings::conflictColor();
    localChangeColor = CervisiaSettings::localChangeColor();
    remoteChangeColor = CervisiaSettings::remoteChangeColor();
}


ProtocolView::~ProtocolView()
{
}


void ProtocolView::updatePlugin(Cervisia::PluginBase* plugin)
{
    kdDebug(8050) << k_funcinfo << endl;

    // make sure we don't connect to the signal twice
    disconnect(plugin, SIGNAL(commandPrepared(Cervisia::CommandBase*)),
               this, 0);

    connect(plugin, SIGNAL(commandPrepared(Cervisia::CommandBase*)),
            this, SLOT(commandPrepared(Cervisia::CommandBase*)));
}


bool ProtocolView::startJob(bool isUpdateJob)
{
//     m_isUpdateJob = isUpdateJob;
//
//     // get command line and add it to output buffer
//     QString cmdLine = job->cvsCommand();
//     buf += cmdLine;
//     buf += '\n';
//     processOutput();
//
//     // disconnect 3rd party slots from our signals
//     disconnect( SIGNAL(receivedLine(QString)) );
//     disconnect( SIGNAL(jobFinished(bool, int)) );
//
//     return job->execute();
}


QPopupMenu* ProtocolView::createPopupMenu(const QPoint &pos)
{
    QPopupMenu* menu = QTextEdit::createPopupMenu(pos);

    int id = menu->insertItem(i18n("Clear"), this, SLOT( clear() ), 0, -1, 0);

    if( length() == 0 )
        menu->setItemEnabled(id, false);

    return menu;
}


void ProtocolView::cancelJob()
{
    m_currentCmd->cancel();
}


void ProtocolView::receivedOutput(const QString& buffer)
{
//    buf += buffer;
//    processOutput();
}


// void ProtocolView::prepareJob(Cervisia::PluginJobBase* job)
// {
//     kdDebug(8050) << "ProtocolView::prepareJob()" << endl;
// 
//     m_isUpdateJob = ((job->action() == Cervisia::PluginJobBase::Update) ||
//                      (job->action() == Cervisia::PluginJobBase::SimulateUpdate));
// 
//     // get command line and add it to output buffer
//     appendLine(job->commandString());
// 
//     // disconnect 3rd party slots from our signals
//     disconnect( SIGNAL(receivedLine(QString)) );
//     disconnect( SIGNAL(jobFinished(bool, int)) );
// 
//     connect(job, SIGNAL(jobExited(bool, int)),
//             this, SLOT(jobExited(bool, int)));
//     connect(job, SIGNAL(receivedLine(const QString&)),
//             this, SLOT(appendLine(const QString&)));
// }


void ProtocolView::commandPrepared(Cervisia::CommandBase* cmd)
{
    kdDebug(8050) << "ProtocolView::commandPrepared()" << endl;

    // not interesting for us?
    if( cmd->action() == Cervisia::CommandBase::Other )
        return;

    m_currentCmd = cmd;

    // get command line and add it to output buffer
    appendLine(cmd->commandString());

    // connect to the command signals
    connect(cmd, SIGNAL(jobExited(bool, int)),
            this, SLOT(jobExited(bool, int)));
    connect(cmd, SIGNAL(receivedLine(const QString&)),
            this, SLOT(appendLine(const QString&)));
}


void ProtocolView::jobExited(bool normalExit, int exitStatus)
{
    QString msg;

    if( normalExit )
    {
        if( exitStatus )
            msg = i18n("[Exited with status %1]\n").arg(exitStatus);
        else
            msg = i18n("[Finished]\n");
    }
    else
        msg = i18n("[Aborted]\n");

    appendLine(msg);

    m_currentCmd = 0;

    emit jobFinished(normalExit, exitStatus);
}


void ProtocolView::appendLine(const QString &line)
{
    // Escape output line, so that html tags in commit
    // messages aren't interpreted
    const QString escapedLine = QStyleSheet::escape(line);

    // When we don't get the output from an update job then
    // just add it to the text edit.
    if( !m_isUpdateJob )
    {
        append(escapedLine);
        return;
    }

    QColor color;
    // Colors are the same as in UpdateViewItem::paintCell()
    if (line.startsWith("C "))
        color = conflictColor;
    else if (line.startsWith("M ")
             || line.startsWith("A ") || line.startsWith("R "))
        color = localChangeColor;
    else if (line.startsWith("P ") || line.startsWith("U "))
        color = remoteChangeColor;

    append(color.isValid()
           ? QString("<font color=\"%1\"><b>%2</b></font>").arg(color.name())
                                                           .arg(escapedLine)
           : escapedLine);
}


#include "protocolview.moc"


// Local Variables:
// c-basic-offset: 4
// End:


