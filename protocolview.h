/*
 *  Copyright (C) 1999-2002 Bernd Gehrmann
 *                          bernd@mail.berlios.de
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


#ifndef PROTOCOLVIEW_H
#define PROTOCOLVIEW_H

#include <qtextedit.h>

namespace Cervisia {
class CommandBase;
class PluginBase;
}

class QPoint;
class QPopupMenu;
class CvsJob_stub;


class ProtocolView : public QTextEdit
{
    Q_OBJECT

public:
    explicit ProtocolView(QWidget *parent=0, const char *name=0);
    ~ProtocolView();

    void updatePlugin(Cervisia::PluginBase* plugin); //TODO: use signal/slot instead?
    bool startJob(bool isUpdateJob = false);

protected:
    virtual QPopupMenu* createPopupMenu(const QPoint &pos);

signals:                                            //TODO: remove later
    void receivedLine(QString line);
    void jobFinished(bool normalExit, int exitStatus);

private slots:
    void receivedOutput(const QString& buffer);     //TODO: remove later
    void appendLine(const QString &line);
//     void prepareJob(Cervisia::PluginJobBase* job);
    void commandPrepared(Cervisia::CommandBase* cmd);
    void jobExited(bool normalExit, int exitStatus);
    void cancelJob();

private:
    QColor conflictColor;
    QColor localChangeColor;
    QColor remoteChangeColor;

    bool   m_isUpdateJob;

    Cervisia::CommandBase* m_currentCmd;
};

#endif


// Local Variables:
// c-basic-offset: 4
// End:
