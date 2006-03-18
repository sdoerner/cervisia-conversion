/*
 *  Copyright (c) 1999-2002 Bernd Gehrmann <bernd@mail.berlios.de>
 *  Copyright (c) 2002-2005 Christian Loose <christian.loose@kdemail.net>
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

#include "progressdlg.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qtimer.h>
#include <qvbox.h>

#include <kanimwidget.h>
#include <kconfig.h>

#include "cervisiasettings.h"
#include "commandbase.h"


struct ProgressDialog::Private
{
    bool                   isCancelled;
    bool                   isShown;
    bool                   hasError;

    QTimer*                timer;
    KAnimWidget*           gear;
    QListBox*              resultbox;
    Cervisia::CommandBase* cmd;
};


ProgressDialog::ProgressDialog(QWidget* parent, const QString& heading,
                               const QString& caption)
    : KDialogBase(parent, 0, true, caption, Ok | Cancel, Cancel, true)
    , d(new Private)
{
    setWFlags(Qt::WDestructiveClose | getWFlags());

    // initialize private data
    d->isCancelled = false;
    d->isShown     = false;
    d->hasError    = false;

    setupGui(heading);
}


ProgressDialog::~ProgressDialog()
{
    delete d;
}


void ProgressDialog::setupGui(const QString& heading)
{
    QVBox* vbox = makeVBoxMainWidget();
    vbox->setSpacing(10);

    QWidget* headingBox = new QWidget(vbox);
    QHBoxLayout* hboxLayout = new QHBoxLayout(headingBox);

    QLabel* textLabel = new QLabel(heading, headingBox);
    textLabel->setMinimumWidth(textLabel->sizeHint().width());
    textLabel->setFixedHeight(textLabel->sizeHint().height());
    hboxLayout->addWidget(textLabel);
    hboxLayout->addStretch();

    d->gear = new KAnimWidget(QString("kde"), 32, headingBox);
    d->gear->setFixedSize(32, 32);
    hboxLayout->addWidget(d->gear);

    d->resultbox = new QListBox(vbox);
    d->resultbox->setSelectionMode(QListBox::NoSelection);
    QFontMetrics fm(d->resultbox->fontMetrics());
    d->resultbox->setMinimumSize(fm.width("0")*70, fm.lineSpacing()*8);

    // only show the cancel button
    showButtonOK(false);
    showButtonCancel(true);

    resize(sizeHint());
}


void ProgressDialog::execute(Cervisia::CommandBase* cmd)
{
    // get command line and display it
    d->resultbox->insertItem(cmd->commandString());

    // establish connections to the signals of the cvs job
    connect(cmd, SIGNAL(jobExited(bool, int)),
            this, SLOT(jobExited(bool, int)));
    connect(cmd, SIGNAL(receivedLine(const QString&)),
            this, SLOT(receivedOutputNonGui(const QString&)));

    // we wait for 4 seconds (or the timeout set by the user) before we
    // force the dialog to show up
    d->timer = new QTimer(this);
    connect(d->timer, SIGNAL(timeout()), this, SLOT(slotTimeoutOccurred()));
    d->timer->start(CervisiaSettings::timeout(), true);

    d->cmd = cmd;

    QApplication::setOverrideCursor(waitCursor);
}


void ProgressDialog::receivedOutputNonGui(const QString& line)
{
    processOutput(line);

    // in case of an error, show the dialog
    if( d->hasError )
    {
        stopNonGuiPart();
        startGuiPart();
    }
}


void ProgressDialog::receivedOutput(const QString& line)
{
    processOutput(line);
}


void ProgressDialog::jobExited(bool normalExit, int status)
{
    Q_UNUSED(normalExit)
    Q_UNUSED(status)

    // don't expect the command to still exist
    d->cmd = 0;

    if( !d->isShown )
        stopNonGuiPart();

    d->gear->stop();

    if( QApplication::overrideCursor() )
        QApplication::restoreOverrideCursor();

    // Close the dialog automatically if there are no
    // error messages or the process has been aborted
    // 'by hand' (e.g.  by clicking the cancel button)
//     if( !d->hasError || !normalExit )
    if( !d->hasError || d->isCancelled )
        accept();
    else
    {
        showButtonOK(true);
        showButtonCancel(false);
    }
}


void ProgressDialog::slotOk()
{
    reject();
}


void ProgressDialog::slotCancel()
{
//     if( !d->isCancelled && d->cmd->isRunning() )
    if( d->cmd->isRunning() )
    {
        d->cmd->cancel();
        d->isCancelled = true;
    }
    else
        reject();
}


void ProgressDialog::slotTimeoutOccurred()
{
    stopNonGuiPart();
    startGuiPart();
}


void ProgressDialog::stopNonGuiPart()
{
//     kdDebug() << "ProgressDialog::stopNonGuiPart()" << endl;

    d->timer->stop();

    disconnect(d->cmd, SIGNAL(receivedLine(const QString&)),
               this, SLOT(receivedOutputNonGui(const QString&)));
}


void ProgressDialog::startGuiPart()
{
//     kdDebug() << "ProgressDialog::startGuiPart()" << endl;

    connect(d->cmd, SIGNAL(receivedLine(const QString&)),
            this, SLOT(receivedOutput(const QString&)));

    show();
    d->isShown = true;

    d->gear->start();
    QApplication::restoreOverrideCursor();
}


void ProgressDialog::processOutput(const QString& line)
{
    if( d->cmd->isErrorMessage(line) )
    {
        d->hasError = true;
        d->resultbox->insertItem(line);
    }
    else if( line.startsWith("cvs server:") )
        d->resultbox->insertItem(line);
}


#include "progressdlg.moc"
