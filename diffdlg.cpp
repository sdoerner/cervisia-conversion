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

#include "diffdlg.h"

#include <tqpushbutton.h>
#include <tqcheckbox.h>
#include <tqcombobox.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqkeycode.h>
#include <tqfileinfo.h>
#include <tqregexp.h>
#include <kconfig.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ktempfile.h>
#include <kprocess.h>

#include "cvsservice_stub.h"
#include "repository_stub.h"
#include "misc.h"
#include "progressdlg.h"
#include "diffview.h"


DiffDialog::DiffDialog(KConfig& cfg, TQWidget *parent, const char *name, bool modal)
    : KDialogBase(parent, name, modal, TQString(),
                  Close | Help | User1, Close, true, KStdGuiItem::saveAs())
    , partConfig(cfg)
{
    items.setAutoDelete(true);
    markeditem = -1;

    TQFrame* mainWidget = makeMainWidget();

    TQBoxLayout *tqlayout = new TQVBoxLayout(mainWidget, 0, spacingHint());

    TQGridLayout *pairtqlayout = new TQGridLayout(tqlayout);
    pairtqlayout->setRowStretch(0, 0);
    pairtqlayout->setRowStretch(1, 1);
    pairtqlayout->setColStretch(1, 0);
    pairtqlayout->addColSpacing(1, 16);
    pairtqlayout->setColStretch(0, 10);
    pairtqlayout->setColStretch(2, 10);

    revlabel1 = new TQLabel(mainWidget);
    pairtqlayout->addWidget(revlabel1, 0, 0);

    revlabel2 = new TQLabel(mainWidget);
    pairtqlayout->addWidget(revlabel2, 0, 2);

    diff1 = new DiffView(cfg, true, false, mainWidget);
    diff2 = new DiffView(cfg, true, true, mainWidget);
    DiffZoomWidget *zoom = new DiffZoomWidget(cfg, mainWidget);
    zoom->setDiffView(diff2);

    pairtqlayout->addWidget(diff1, 1, 0);
    pairtqlayout->addWidget(zoom,  1, 1);
    pairtqlayout->addWidget(diff2, 1, 2);

    diff1->setPartner(diff2);
    diff2->setPartner(diff1);

    syncbox = new TQCheckBox(i18n("Synchronize scroll bars"), mainWidget);
    syncbox->setChecked(true);
    connect( syncbox, TQT_SIGNAL(toggled(bool)),
	     this, TQT_SLOT(toggleSynchronize(bool)) );

    itemscombo = new TQComboBox(mainWidget);
    itemscombo->insertItem(TQString());
    connect( itemscombo, TQT_SIGNAL(activated(int)),
             this, TQT_SLOT(comboActivated(int)) );

    nofnlabel = new TQLabel(mainWidget);
    // avoids auto resize when the text is changed
    nofnlabel->setMinimumWidth(fontMetrics().width(i18n("%1 differences").tqarg(10000)));

    backbutton = new TQPushButton(TQString::tqfromLatin1("&<<"), mainWidget);
    connect( backbutton, TQT_SIGNAL(clicked()), TQT_SLOT(backClicked()) );

    forwbutton = new TQPushButton(TQString::tqfromLatin1("&>>"), mainWidget);
    connect( forwbutton, TQT_SIGNAL(clicked()), TQT_SLOT(forwClicked()) );

    connect( this, TQT_SIGNAL(user1Clicked()), TQT_SLOT(saveAsClicked()) );

    TQBoxLayout *buttontqlayout = new TQHBoxLayout(tqlayout);
    buttontqlayout->addWidget(syncbox, 0);
    buttontqlayout->addStretch(4);
    buttontqlayout->addWidget(itemscombo);
    buttontqlayout->addStretch(1);
    buttontqlayout->addWidget(nofnlabel);
    buttontqlayout->addStretch(1);
    buttontqlayout->addWidget(backbutton);
    buttontqlayout->addWidget(forwbutton);

    setHelp("diff");

    setWFlags(TQt::WDestructiveClose | getWFlags());

    TQSize size = configDialogSize(partConfig, "DiffDialog");
    resize(size);

    KConfigGroupSaver cs(&partConfig, "DiffDialog");
    syncbox->setChecked(partConfig.readBoolEntry("Sync"));
}


DiffDialog::~DiffDialog()
{
    saveDialogSize(partConfig, "DiffDialog");

    KConfigGroupSaver cs(&partConfig, "DiffDialog");
    partConfig.writeEntry("Sync", syncbox->isChecked());
}


void DiffDialog::keyPressEvent(TQKeyEvent *e)
{
    switch (e->key())
	{
	case Key_Up:
            diff1->up();
            diff2->up();
            break;
	case Key_Down:
            diff1->down();
            diff2->down();
            break;
	case Key_Next:
            diff1->next();
            diff2->next();
            break;
	case Key_Prior:
            diff1->prior();
            diff2->prior();
            break;
        default:
            KDialogBase::keyPressEvent(e);
	}
}


void DiffDialog::toggleSynchronize(bool b)
{
    diff1->setPartner(b? diff2 : 0);
    diff2->setPartner(b? diff1 : 0);
}


void DiffDialog::comboActivated(int index)
{
    updateHighlight(index-1);
}


static void interpretRegion(TQString line, int *linenoA, int *linenoB)
{
    TQRegExp region( "^@@ -([0-9]+),([0-9]+) \\+([0-9]+),([0-9]+) @@.*$" );

    if (!region.exactMatch(line))
        return;

    *linenoA = region.cap(1).toInt() - 1;
    *linenoB = region.cap(3).toInt() - 1;
}


static TQString regionAsString(int linenoA, int linecountA, int linenoB, int linecountB)
{
    int lineendA = linenoA+linecountA-1;
    int lineendB = linenoB+linecountB-1;
    TQString res;
    if (linecountB == 0)
        res = TQString("%1,%2d%3").tqarg(linenoA).tqarg(lineendA).tqarg(linenoB-1);
    else if (linecountA == 0)
        res = TQString("%1a%2,%3").tqarg(linenoA-1).tqarg(linenoB).tqarg(lineendB);
    else if (linenoA == lineendA)
        if (linenoB == lineendB)
            res = TQString("%1c%2").tqarg(linenoA).tqarg(linenoB);
        else
            res = TQString("%1c%2,%3").tqarg(linenoA).tqarg(linenoB).tqarg(lineendB);
    else if (linenoB == lineendB)
        res = TQString("%1,%2c%3").tqarg(linenoA).tqarg(lineendA).tqarg(linenoB);
    else
        res = TQString("%1,%2c%3,%4").tqarg(linenoA).tqarg(lineendA).tqarg(linenoB).tqarg(lineendB);

    return res;

}


class DiffItem
{
public:
    DiffView::DiffType type;
    int linenoA, linecountA;
    int linenoB, linecountB;
};


bool DiffDialog::parseCvsDiff(CvsService_stub* service, const TQString& fileName,
                              const TQString &revA, const TQString &revB)
{
    TQStringList linesA, linesB;
    int linenoA, linenoB;

    setCaption(i18n("CVS Diff: %1").tqarg(fileName));
    revlabel1->setText( revA.isEmpty()?
                        i18n("Repository:")
                        : i18n("Revision ")+revA+":" );
    revlabel2->setText( revB.isEmpty()?
                        i18n("Working dir:")
                        : i18n("Revision ")+revB+":" );

    KConfigGroupSaver cs(&partConfig, "General");

    // Ok, this is a hack: When the user wants an external diff
    // front end, it is executed from here. Of course, in that
    // case this dialog wouldn't have to be created in the first
    // place, but this design at least makes the handling trans-
    // parent for the calling routines

    TQString extdiff = partConfig.readPathEntry("ExternalDiff");
    if (!extdiff.isEmpty())
        {
            callExternalDiff(extdiff, service, fileName, revA, revB);
            return false;
        }

    const TQString diffOptions   = partConfig.readEntry("DiffOptions");
    const unsigned contextLines = partConfig.readUnsignedNumEntry("ContextLines", 65535);

    DCOPRef job = service->diff(fileName, revA, revB, diffOptions, contextLines);
    if( !service->ok() )
        return false;

    ProgressDialog dlg(this, "Diff", job, "diff", i18n("CVS Diff"));
    if( !dlg.execute() )
        return false;

    // remember diff output for "save as" action
    m_diffOutput = dlg.getOutput();

    TQString line;
    while ( dlg.getLine(line) && !line.startsWith("+++"))
        ;

    linenoA = linenoB = 0;
    while ( dlg.getLine(line) )
    {
        // line contains diff region?
        if (line.startsWith("@@"))
        {
            interpretRegion(line, &linenoA, &linenoB);
            diff1->addLine(line, DiffView::Separator);
            diff2->addLine(line, DiffView::Separator);
            continue;
        }

        if (line.length() < 1)
            continue;

        TQChar marker = line[0];
        line.remove(0, 1);

        if (marker == '-')
            linesA.append(line);
        else if (marker == '+')
            linesB.append(line);
        else
        {
            if (!linesA.isEmpty() || !linesB.isEmpty())
            {
                newDiffHunk(linenoA, linenoB, linesA, linesB);

                linesA.clear();
                linesB.clear();
            }
            diff1->addLine(line, DiffView::Unchanged, ++linenoA);
            diff2->addLine(line, DiffView::Unchanged, ++linenoB);
        }
    }

    if (!linesA.isEmpty() || !linesB.isEmpty())
        newDiffHunk(linenoA, linenoB, linesA, linesB);

    // sets the right size as there is no more auto resize in TQComboBox
    itemscombo->adjustSize();

    updateNofN();

    return true;
}


void DiffDialog::newDiffHunk(int& linenoA, int& linenoB,
                             const TQStringList& linesA, const TQStringList& linesB)
{
    DiffItem *item = new DiffItem;
    item->linenoA    = linenoA+1;
    item->linenoB    = linenoB+1;
    item->linecountA = linesA.count();
    item->linecountB = linesB.count();
    items.append(item);

    const TQString region = regionAsString(linenoA+1, linesA.count(),
                                          linenoB+1, linesB.count());
    itemscombo->insertItem(region);

    TQStringList::ConstIterator itA = linesA.begin();
    TQStringList::ConstIterator itB = linesB.begin();
    while (itA != linesA.end() || itB != linesB.end())
    {
        if (itA != linesA.end())
        {
            diff1->addLine(*itA, DiffView::Neutral, ++linenoA);
            if (itB != linesB.end())
                diff2->addLine(*itB, DiffView::Change, ++linenoB);
            else
                diff2->addLine("", DiffView::Delete);
        }
        else
        {
            diff1->addLine("", DiffView::Neutral);
            diff2->addLine(*itB, DiffView::Insert, ++linenoB);
        }

        if (itA != linesA.end())
            ++itA;
        if (itB != linesB.end())
            ++itB;
    }
}


void DiffDialog::callExternalDiff(const TQString& extdiff, CvsService_stub* service,
                                  const TQString& fileName, const TQString &revA,
                                  const TQString &revB)
{
    TQString extcmdline = extdiff;
    extcmdline += " ";

    // create suffix for temporary file (used TQFileInfo to remove path from file name)
    const TQString suffix = "-" + TQFileInfo(fileName).fileName();

    DCOPRef job;
    if (!revA.isEmpty() && !revB.isEmpty())
    {
        // We're comparing two revisions
        TQString revAFilename = tempFileName(suffix+TQString("-")+revA);
        TQString revBFilename = tempFileName(suffix+TQString("-")+revB);

        // download the files for revision A and B
        job = service->downloadRevision(fileName, revA, revAFilename,
                                                revB, revBFilename);
        if( !service->ok() )
            return;

        extcmdline += KProcess::quote(revAFilename);
        extcmdline += " ";
        extcmdline += KProcess::quote(revBFilename);
    }
    else
    {
        // We're comparing to a file, and perhaps one revision
        TQString revAFilename = tempFileName(suffix+TQString("-")+revA);
        job = service->downloadRevision(fileName, revA, revAFilename);
        if( !service->ok() )
            return;

        extcmdline += KProcess::quote(revAFilename);
        extcmdline += " ";
        extcmdline += KProcess::quote(TQFileInfo(fileName).absFilePath());
    }

    ProgressDialog dlg(this, "Diff", job, "diff");
    if( dlg.execute() )
    {
        // call external diff application
        // TODO CL maybe use system()?
        KProcess proc;
        proc.setUseShell(true, "/bin/sh");
        proc << extcmdline;
        proc.start(KProcess::DontCare);
    }
}


void DiffDialog::updateNofN()
{
    TQString str;
    if (markeditem >= 0)
	str = i18n("%1 of %2").tqarg(markeditem+1).tqarg(items.count());
    else
	str = i18n("%1 differences").tqarg(items.count());
    nofnlabel->setText(str);

    itemscombo->setCurrentItem(markeditem==-2? 0 : markeditem+1);

    backbutton->setEnabled(markeditem != -1);
    forwbutton->setEnabled(markeditem != -2 && items.count());
}


void DiffDialog::updateHighlight(int newitem)
{
    if (markeditem >= 0)
	{
	    DiffItem *item = items.at(markeditem);
	    for (int i = item->linenoA; i < item->linenoA+item->linecountA; ++i)
		diff1->setInverted(i, false);
	    for (int i = item->linenoB; i < item->linenoB+item->linecountB; ++i)
		diff2->setInverted(i, false);
	}

    markeditem = newitem;

    if (markeditem >= 0)
	{
	    DiffItem *item = items.at(markeditem);
	    for (int i = item->linenoA; i < item->linenoA+item->linecountA; ++i)
		diff1->setInverted(i, true);
	    for (int i = item->linenoB; i < item->linenoB+item->linecountB; ++i)
		diff2->setInverted(i, true);
	    diff1->setCenterLine(item->linenoA);
	    diff2->setCenterLine(item->linenoB);
	}
    diff1->tqrepaint();
    diff2->tqrepaint();
    updateNofN();
}


void DiffDialog::backClicked()
{
    int newitem;
    if (markeditem == -1)
        return; // internal error (button not disabled)
    else if (markeditem == -2) // past end
        newitem = items.count()-1;
    else
        newitem = markeditem-1;
    updateHighlight(newitem);
}


void DiffDialog::forwClicked()
{
    int newitem;
    if (markeditem == -2 || (markeditem == -1 && !items.count()))
        return; // internal error (button not disabled)
    else if (markeditem+1 == (int)items.count()) // past end
        newitem = -2;
    else
        newitem = markeditem+1;
    updateHighlight(newitem);
}


void DiffDialog::saveAsClicked()
{
    TQString fileName = KFileDialog::getSaveFileName(TQString(), TQString(), this);
    if( fileName.isEmpty() )
        return;

    if( !Cervisia::CheckOverwrite(fileName, this) )
        return;

    TQFile f(fileName);
    if( !f.open(IO_WriteOnly) )
    {
        KMessageBox::sorry(this,
                           i18n("Could not open file for writing."),
                           "Cervisia");
        return;
    }

    TQTextStream ts(&f);
    TQStringList::Iterator it = m_diffOutput.begin();
    for( ; it != m_diffOutput.end(); ++it )
        ts << *it << "\n";

    f.close();
}

#include "diffdlg.moc"


// Local Variables:
// c-basic-offset: 4
// End:
