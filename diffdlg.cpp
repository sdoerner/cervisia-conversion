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

#include "diffdlg.h"

#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qkeycode.h>
#include <qfileinfo.h>
#include <qregexp.h>
#include <kconfig.h>
#include <klocale.h>
#include <ktempfile.h>
#include <kprocess.h>

#include "cvsservice_stub.h"
#include "repository_stub.h"
#include "misc.h"
#include "cvsprogressdlg.h"
#include "progressdlg.h"
#include "diffview.h"

#include <kdeversion.h>
#if KDE_VERSION < KDE_MAKE_VERSION(3,1,90)
#include "configutils.h"
#endif


DiffDialog::DiffDialog(KConfig& cfg, QWidget *parent, const char *name, bool modal)
    : KDialogBase(parent, name, modal, QString::null,
                  Close | Help, Close, true)
    , partConfig(cfg)
{
    items.setAutoDelete(true);
    markeditem = -1;

    QFrame* mainWidget = makeMainWidget();

    QBoxLayout *layout = new QVBoxLayout(mainWidget, 0, spacingHint());

    QGridLayout *pairlayout = new QGridLayout(layout);
    pairlayout->setRowStretch(0, 0);
    pairlayout->setRowStretch(1, 1);
    pairlayout->setColStretch(1, 0);
    pairlayout->addColSpacing(1, 16);
    pairlayout->setColStretch(0, 10);
    pairlayout->setColStretch(2, 10);

    revlabel1 = new QLabel(mainWidget);
    pairlayout->addWidget(revlabel1, 0, 0);
			      
    revlabel2 = new QLabel(mainWidget);
    pairlayout->addWidget(revlabel2, 0, 2);

    diff1 = new DiffView(true, false, mainWidget);
    diff2 = new DiffView(true, true, mainWidget);
    DiffZoomWidget *zoom = new DiffZoomWidget(mainWidget);
    zoom->setDiffView(diff2);

    pairlayout->addWidget(diff1, 1, 0);
    pairlayout->addWidget(zoom,  1, 1);
    pairlayout->addWidget(diff2, 1, 2);

    diff1->setPartner(diff2);
    diff2->setPartner(diff1);
    
    syncbox = new QCheckBox(i18n("Synchronize scroll bars"), mainWidget);
    syncbox->setChecked(true);
    connect( syncbox, SIGNAL(toggled(bool)),
	     this, SLOT(toggleSynchronize(bool)) );

    itemscombo = new QComboBox(mainWidget);
    itemscombo->insertItem(QString::null);
    connect( itemscombo, SIGNAL(activated(int)),
             this, SLOT(comboActivated(int)) );
    
    nofnlabel = new QLabel(mainWidget);
    // avoids auto resize when the text is changed
    nofnlabel->setMinimumWidth(fontMetrics().width(i18n("%1 differences").arg(10000)));
    
    backbutton = new QPushButton(QString::fromLatin1("&<<"), mainWidget);
    connect( backbutton, SIGNAL(clicked()), SLOT(backClicked()) );
    
    forwbutton = new QPushButton(QString::fromLatin1("&>>"), mainWidget);
    connect( forwbutton, SIGNAL(clicked()), SLOT(forwClicked()) );

    QBoxLayout *buttonlayout = new QHBoxLayout(layout);
    buttonlayout->addWidget(syncbox, 0);
    buttonlayout->addStretch(4);
    buttonlayout->addWidget(itemscombo);
    buttonlayout->addStretch(1);
    buttonlayout->addWidget(nofnlabel);
    buttonlayout->addStretch(1);
    buttonlayout->addWidget(backbutton);
    buttonlayout->addWidget(forwbutton);

    setHelp("diff");

    setWFlags(Qt::WDestructiveClose | getWFlags());

#if KDE_IS_VERSION(3,1,90)
    QSize size = configDialogSize(partConfig, "DiffDialog");
#else
    QSize size = Cervisia::configDialogSize(this, partConfig, "DiffDialog");
#endif
    resize(size);

    KConfigGroupSaver cs(&partConfig, "DiffDialog");
    syncbox->setChecked(partConfig.readBoolEntry("Sync"));
}


DiffDialog::~DiffDialog()
{
#if KDE_IS_VERSION(3,1,90)
    saveDialogSize(partConfig, "DiffDialog");
#else
    Cervisia::saveDialogSize(this, partConfig, "DiffDialog");
#endif

    KConfigGroupSaver cs(&partConfig, "DiffDialog");
    partConfig.writeEntry("Sync", syncbox->isChecked());
}


void DiffDialog::keyPressEvent(QKeyEvent *e)
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


static void interpretRegion(QString line, int *linenoA, int *linenoB)
{   
    QRegExp region( "^@@ -([0-9]+),([0-9]+) \\+([0-9]+),([0-9]+) @@.*$" );
    
    if (!region.exactMatch(line))
        return;

    *linenoA = region.cap(1).toInt() - 1;
    *linenoB = region.cap(3).toInt() - 1;   
}


static QString regionAsString(int linenoA, int linecountA, int linenoB, int linecountB)
{
    int lineendA = linenoA+linecountA-1;
    int lineendB = linenoB+linecountB-1;
    QString res;
    if (linecountB == 0)
        res = QString("%1,%2d%3").arg(linenoA).arg(lineendA).arg(linenoB-1);
    else if (linecountA == 0)
        res = QString("%1a%2,%3").arg(linenoA-1).arg(linenoB).arg(lineendB);
    else if (linenoA == lineendA)
        if (linenoB == lineendB)
            res = QString("%1c%2").arg(linenoA).arg(linenoB);
        else 
            res = QString("%1c%2,%3").arg(linenoA).arg(linenoB).arg(lineendB);
    else if (linenoB == lineendB)
        res = QString("%1,%2c%3").arg(linenoA).arg(lineendA).arg(linenoB);
    else
        res = QString("%1,%2c%3,%4").arg(linenoA).arg(lineendA).arg(linenoB).arg(lineendB);

    return res;
    
}


class DiffItem
{
public:
    DiffView::DiffType type;
    int linenoA, linecountA;
    int linenoB, linecountB;
};


bool DiffDialog::parseCvsDiff(CvsService_stub* service, const QString& fileName,
                              const QString &revA, const QString &revB)
{
    QStringList linesA, linesB;
    int linenoA, linenoB;

    setCaption(i18n("CVS Diff: %1").arg(fileName));
    revlabel1->setText( revA.isEmpty()?
                        i18n("Repository")
                        : i18n("Revision ")+revA );
    revlabel2->setText( revB.isEmpty()?
                        i18n("Working dir")
                        : i18n("Revision ")+revB );
    
    KConfigGroupSaver cs(&partConfig, "General");

    // Ok, this is a hack: When the user wants an external diff
    // front end, it is executed from here. Of course, in that
    // case this dialog wouldn't have to be created in the first
    // place, but this design at least makes the handling trans-
    // parent for the calling routines

    QString extdiff = partConfig.readEntry("ExternalDiff", "");
    if (!extdiff.isEmpty())
        {
            callExternalDiff(extdiff, service, fileName, revA, revB);
            return false;
        }

    const QString diffOptions   = partConfig.readEntry("DiffOptions", "");
    const unsigned contextLines = partConfig.readUnsignedNumEntry("ContextLines", 65535);
        
    DCOPRef job = service->diff(fileName, revA, revB, diffOptions, contextLines);
    if( !service->ok() )
        return false;

    ProgressDialog dlg(this, "Diff", job, "diff", i18n("CVS Diff"));
    if( !dlg.execute() )
        return false;
    
    QString line;
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
            
        QChar marker = line[0];
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

    // sets the right size as there is no more auto resize in QComboBox
    itemscombo->adjustSize();

    updateNofN();
 
    return true;
}


void DiffDialog::newDiffHunk(int& linenoA, int& linenoB, 
                             const QStringList& linesA, const QStringList& linesB)
{
    DiffItem *item = new DiffItem;
    item->linenoA    = linenoA+1;
    item->linenoB    = linenoB+1;
    item->linecountA = linesA.count();
    item->linecountB = linesB.count();
    items.append(item);
                        
    const QString region = regionAsString(linenoA+1, linesA.count(),
                                          linenoB+1, linesB.count());
    itemscombo->insertItem(region);
                                    
    QStringList::ConstIterator itA = linesA.begin();
    QStringList::ConstIterator itB = linesB.begin();
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


void DiffDialog::callExternalDiff(const QString& extdiff, CvsService_stub* service, 
                                  const QString& fileName, const QString &revA,
                                  const QString &revB)
{
    QString cmdline = "cvs update -p ";
    QString extcmdline = extdiff;
    extcmdline += " ";
            
    if (!revA.isEmpty() && !revB.isEmpty())
    {
        // We're comparing two revisions
        QString revAFilename = tempFileName(QString("-")+revA);
        QString revBFilename = tempFileName(QString("-")+revB);
                    
        cmdline += " -r ";
        cmdline += KProcess::quote(revA);
        cmdline += " ";
        cmdline += KProcess::quote(fileName);
        cmdline += " > ";
        cmdline += KProcess::quote(revAFilename);
        cmdline += " ; cvs update -p ";
        cmdline += " -r ";
        cmdline += KProcess::quote(revB);
        cmdline += " ";
        cmdline += KProcess::quote(fileName);
        cmdline += " > ";
        cmdline += KProcess::quote(revBFilename);
                    
        extcmdline += KProcess::quote(revAFilename);
        extcmdline += " ";
        extcmdline += KProcess::quote(revBFilename);
    } 
    else 
    {
        // We're comparing to a file, and perhaps one revision
        QString revAFilename = tempFileName(revA);
        if (!revA.isEmpty())
        {
            cmdline += " -r ";
            cmdline += KProcess::quote(revA);
        }
        cmdline += " ";
        cmdline += KProcess::quote(fileName);
        cmdline += " > ";
        cmdline += KProcess::quote(revAFilename);
                    
        extcmdline += KProcess::quote(revAFilename);
        extcmdline += " ";
        extcmdline += KProcess::quote(QFileInfo(fileName).absFilePath());
    }

    // FIXME: We shouldn't need to use CvsProgressDialog here
    Repository_stub cvsRepository(service->app(), "CvsRepository");
    QString sandbox    = cvsRepository.workingCopy();
    QString repository = cvsRepository.location();
    
    CvsProgressDialog l("Diff", this);
    if (l.execCommand(sandbox, repository, cmdline, "diff"))
    {
        KProcess proc;
        proc.setUseShell(true, "/bin/sh");
        proc << extcmdline;
        proc.start(KProcess::DontCare);
    }
            
}


void DiffDialog::updateNofN()
{
    QString str;
    if (markeditem >= 0)
	str = i18n("%1 of %2").arg(markeditem+1).arg(items.count());
    else
	str = i18n("%1 differences").arg(items.count());
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
    diff1->repaint();
    diff2->repaint();
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


#include "diffdlg.moc"


// Local Variables:
// c-basic-offset: 4
// End:
