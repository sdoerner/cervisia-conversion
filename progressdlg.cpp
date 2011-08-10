/* 
 *  Copyright (c) 1999-2002 Bernd Gehrmann <bernd@mail.berlios.de>
 *  Copyright (c) 2002-2004 Christian Loose <christian.loose@kdemail.net>
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

#include <tqlabel.h>
#include <tqlayout.h>
#include <tqstring.h>
#include <tqstringlist.h>
#include <tqtimer.h>
#include <tqvbox.h>

#include <cvsjob_stub.h>
#include <dcopref.h>
#include <kanimwidget.h>
#include <kapplication.h>
#include <kconfig.h>

#include "cervisiasettings.h"


struct ProgressDialog::Private
{
    bool            isCancelled;
    bool            isShown;
    bool            hasError;

    CvsJob_stub*    cvsJob;
    TQString         buffer;
    TQString         errorId1, errorId2;
    TQStringList     output;

    TQTimer*         timer;
    KAnimWidget*    gear;
    TQListBox*       resultbox;
};


ProgressDialog::ProgressDialog(TQWidget* tqparent, const TQString& heading,
                               const DCOPRef& job, const TQString& errorIndicator,
                               const TQString& caption)
    : KDialogBase(tqparent, 0, true, caption, Cancel, Cancel, true)
    , DCOPObject()
    , d(new Private)
{
    // initialize private data
    d->isCancelled = false;
    d->isShown     = false;
    d->hasError    = false;

    d->cvsJob      = new CvsJob_stub(job);
    d->buffer      = "";

    d->errorId1 = "cvs " + errorIndicator + ":";
    d->errorId2 = "cvs [" + errorIndicator + " aborted]:";

    setupGui(heading);
}


ProgressDialog::~ProgressDialog()
{
    delete d->cvsJob;
    delete d;
}


void ProgressDialog::setupGui(const TQString& heading)
{
    TQVBox* vbox = makeVBoxMainWidget();
    vbox->setSpacing(10);

    TQWidget* headingBox = new TQWidget(vbox);
    TQHBoxLayout* hboxLayout = new TQHBoxLayout(headingBox);

    TQLabel* textLabel = new TQLabel(heading, headingBox);
    textLabel->setMinimumWidth(textLabel->tqsizeHint().width());
    textLabel->setFixedHeight(textLabel->tqsizeHint().height());
    hboxLayout->addWidget(textLabel);
    hboxLayout->addStretch();

    d->gear = new KAnimWidget(TQString("kde"), 32, headingBox);
    d->gear->setFixedSize(32, 32);
    hboxLayout->addWidget(d->gear);

    d->resultbox = new TQListBox(vbox);
    d->resultbox->setSelectionMode(TQListBox::NoSelection);
    TQFontMetrics fm(d->resultbox->fontMetrics());
    d->resultbox->setMinimumSize(fm.width("0")*70, fm.lineSpacing()*8);

    resize(tqsizeHint());
}


bool ProgressDialog::execute()
{
    // get command line and display it
    TQString cmdLine = d->cvsJob->cvsCommand();
    d->resultbox->insertItem(cmdLine);

    // establish connections to the signals of the cvs job
    connectDCOPSignal(d->cvsJob->app(), d->cvsJob->obj(), "jobExited(bool, int)",
                      "slotJobExited(bool, int)", true);
    connectDCOPSignal(d->cvsJob->app(), d->cvsJob->obj(), "receivedStdout(TQString)",
                      "slotReceivedOutputNonGui(TQString)", true);
    connectDCOPSignal(d->cvsJob->app(), d->cvsJob->obj(), "receivedStderr(TQString)",
                      "slotReceivedOutputNonGui(TQString)", true);

    // we wait for 4 seconds (or the timeout set by the user) before we
    // force the dialog to show up
    d->timer = new TQTimer(this);
    connect(d->timer, TQT_SIGNAL(timeout()), this, TQT_SLOT(slotTimeoutOccurred()));
    d->timer->start(CervisiaSettings::timeout(), true);

    bool started = d->cvsJob->execute();
    if( !started )
        return false;

    TQApplication::setOverrideCursor(waitCursor);
    kapp->enter_loop();
    if (TQApplication::overrideCursor())
        TQApplication::restoreOverrideCursor();

    return !d->isCancelled;
}


bool ProgressDialog::getLine(TQString& line)
{
    if( d->output.isEmpty() )
        return false;

    line = d->output.first();
    d->output.remove(d->output.begin());

    return true;
}


TQStringList ProgressDialog::getOutput() const
{
    return d->output;
}


void ProgressDialog::slotReceivedOutputNonGui(TQString buffer)
{
    d->buffer += buffer;

    processOutput();
    if( d->hasError )
    {
        stopNonGuiPart();
        startGuiPart();
    }
}


void ProgressDialog::slotReceivedOutput(TQString buffer)
{
    d->buffer += buffer;
    processOutput();
}


void ProgressDialog::slotJobExited(bool normalExit, int status)
{
    Q_UNUSED(status)

    if( !d->isShown )
        stopNonGuiPart();

    d->gear->stop();
    if( !d->buffer.isEmpty() )
    {
        d->buffer += '\n';
        processOutput();
    }

    // Close the dialog automatically if there are no
    // error messages or the process has been aborted
    // 'by hand' (e.g.  by clicking the cancel button)
    if( !d->hasError || !normalExit )
        kapp->exit_loop();
}


void ProgressDialog::slotCancel()
{
    d->isCancelled = true;

    bool isRunning = d->cvsJob->isRunning();
    if( isRunning )
        d->cvsJob->cancel();
    else
        kapp->exit_loop();
}


void ProgressDialog::slotTimeoutOccurred()
{
    stopNonGuiPart();
    startGuiPart();
}


void ProgressDialog::stopNonGuiPart()
{
    d->timer->stop();

    disconnectDCOPSignal(d->cvsJob->app(), d->cvsJob->obj(), "receivedStdout(TQString)",
                      "slotReceivedOutputNonGui(TQString)");
    disconnectDCOPSignal(d->cvsJob->app(), d->cvsJob->obj(), "receivedStderr(TQString)",
                      "slotReceivedOutputNonGui(TQString)");

    kapp->exit_loop();
}


void ProgressDialog::startGuiPart()
{
    connectDCOPSignal(d->cvsJob->app(), d->cvsJob->obj(), "receivedStdout(TQString)",
                      "slotReceivedOutput(TQString)", true);
    connectDCOPSignal(d->cvsJob->app(), d->cvsJob->obj(), "receivedStderr(TQString)",
                      "slotReceivedOutput(TQString)", true);

    show();
    d->isShown = true;

    d->gear->start();
    TQApplication::restoreOverrideCursor();
    kapp->enter_loop();
}


void ProgressDialog::processOutput()
{
    int pos;
    while( (pos = d->buffer.find('\n')) != -1 )
    {
        TQString item = d->buffer.left(pos);
        if( item.startsWith(d->errorId1) ||
            item.startsWith(d->errorId2) ||
            item.startsWith("cvs [server aborted]:") )
        {
            d->hasError = true;
            d->resultbox->insertItem(item);
        }
        else if( item.startsWith("cvs server:") )
            d->resultbox->insertItem(item);
        else
            d->output.append(item);

        // remove item from buffer
        d->buffer.remove(0, pos+1);
    }
}


#include "progressdlg.moc"
