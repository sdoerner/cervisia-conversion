/*
 *  Copyright (C) 1999-2002 Bernd Gehrmann
 *                          bernd@mail.berlios.de
 *  Copyright (c) 2003-2004 Christian Loose <christian.loose@hamburg.de>
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


#include "resolvedlg.h"

#include <tqfile.h>
#include <tqkeycode.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqpushbutton.h>
#include <tqtextcodec.h>
#include <tqtextstream.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <tqregexp.h>
#include "misc.h"
#include "resolvedlg_p.h"
using Cervisia::ResolveEditorDialog;


// *UGLY HACK*
// The following conditions are a rough hack
static TQTextCodec *DetectCodec(const TQString &fileName)
{
    if (fileName.endsWith(".ui") || fileName.endsWith(".docbook")
        || fileName.endsWith(".xml"))
        return TQTextCodec::codecForName("utf8");

    return TQTextCodec::codecForLocale();
}


namespace
{

class LineSeparator
{
public:
    LineSeparator(const TQString& text)
        : m_text(text)
        , m_startPos(0)
        , m_endPos(0)
    {
    }
    
    TQString nextLine() const
    {
        // already reach end of text on previous call
        if( m_endPos < 0 )
        {
            m_currentLine = TQString::null;
            return m_currentLine;
        }
        
        m_endPos = m_text.find('\n', m_startPos);
        
        int length    = m_endPos - m_startPos + 1;
        m_currentLine = m_text.mid(m_startPos, length);
        m_startPos    = m_endPos + 1;
        
        return m_currentLine;
    }
    
    bool atEnd() const
    {
        return (m_endPos < 0 && m_currentLine.isEmpty());
    }
    
private:
    const TQString    m_text;
    mutable TQString  m_currentLine;
    mutable int      m_startPos, m_endPos;
};

}


ResolveDialog::ResolveDialog(KConfig& cfg, TQWidget *parent, const char *name)
    : KDialogBase(parent, name, false, TQString::null,
                  Close | Help | User1 | User2, Close, true,
                  KStdGuiItem::saveAs(), KStdGuiItem::save())
    , markeditem(-1)
    , partConfig(cfg)
{
    items.setAutoDelete(true);

    TQFrame* mainWidget = makeMainWidget();

    TQBoxLayout *layout = new TQVBoxLayout(mainWidget, 0, spacingHint());

    TQSplitter *vertSplitter = new TQSplitter(TQSplitter::Vertical, mainWidget);

    TQSplitter *splitter = new TQSplitter(TQSplitter::Horizontal, vertSplitter);

    TQWidget *versionALayoutWidget = new TQWidget(splitter);
    TQBoxLayout *versionAlayout = new TQVBoxLayout(versionALayoutWidget, 5);

    TQLabel *revlabel1 = new TQLabel(i18n("Your version (A):"), versionALayoutWidget);
    versionAlayout->addWidget(revlabel1);
    diff1 = new DiffView(cfg, true, false, versionALayoutWidget);
    versionAlayout->addWidget(diff1, 10);

    TQWidget* versionBLayoutWidget = new TQWidget(splitter);
    TQBoxLayout *versionBlayout = new TQVBoxLayout(versionBLayoutWidget, 5);

    TQLabel *revlabel2 = new TQLabel(i18n("Other version (B):"), versionBLayoutWidget);
    versionBlayout->addWidget(revlabel2);
    diff2 = new DiffView(cfg, true, false, versionBLayoutWidget);
    versionBlayout->addWidget(diff2, 10);

    diff1->setPartner(diff2);
    diff2->setPartner(diff1);

    TQWidget* mergeLayoutWidget = new TQWidget(vertSplitter);
    TQBoxLayout *mergeLayout = new TQVBoxLayout(mergeLayoutWidget, 5);

    TQLabel *mergelabel = new TQLabel(i18n("Merged version:"), mergeLayoutWidget);
    mergeLayout->addWidget(mergelabel);

    merge = new DiffView(cfg, false, false, mergeLayoutWidget);
    mergeLayout->addWidget(merge, 10);

    layout->addWidget(vertSplitter);

    abutton = new TQPushButton("&A", mainWidget);
    connect( abutton, TQT_SIGNAL(clicked()), TQT_SLOT(aClicked()) );

    bbutton = new TQPushButton("&B", mainWidget);
    connect( bbutton, TQT_SIGNAL(clicked()), TQT_SLOT(bClicked()) );

    abbutton = new TQPushButton("A+B", mainWidget);
    connect( abbutton, TQT_SIGNAL(clicked()), TQT_SLOT(abClicked()) );

    babutton = new TQPushButton("B+A", mainWidget);
    connect( babutton, TQT_SIGNAL(clicked()), TQT_SLOT(baClicked()) );

    editbutton = new TQPushButton(i18n("&Edit"), mainWidget);
    connect( editbutton, TQT_SIGNAL(clicked()), TQT_SLOT(editClicked()) );

    nofnlabel = new TQLabel(mainWidget);
    nofnlabel->setAlignment(AlignCenter);

    backbutton = new TQPushButton("&<<", mainWidget);
    connect( backbutton, TQT_SIGNAL(clicked()), TQT_SLOT(backClicked()) );

    forwbutton = new TQPushButton("&>>", mainWidget);
    connect( forwbutton, TQT_SIGNAL(clicked()), TQT_SLOT(forwClicked()) );

    TQBoxLayout *buttonlayout = new TQHBoxLayout(layout);
    buttonlayout->addWidget(abutton, 1);
    buttonlayout->addWidget(bbutton, 1);
    buttonlayout->addWidget(abbutton, 1);
    buttonlayout->addWidget(babutton, 1);
    buttonlayout->addWidget(editbutton, 1);
    buttonlayout->addStretch(1);
    buttonlayout->addWidget(nofnlabel, 2);
    buttonlayout->addStretch(1);
    buttonlayout->addWidget(backbutton, 1);
    buttonlayout->addWidget(forwbutton, 1);

    connect( this, TQT_SIGNAL(user2Clicked()), TQT_SLOT(saveClicked()) );
    connect( this, TQT_SIGNAL(user1Clicked()), TQT_SLOT(saveAsClicked()) );

    TQFontMetrics const fm(fontMetrics());
    setMinimumSize(fm.width('0') * 120,
                   fm.lineSpacing() * 40);

    setHelp("resolvingconflicts");

    setWFlags(Qt::WDestructiveClose | getWFlags());

    TQSize size = configDialogSize(partConfig, "ResolveDialog");
    resize(size);
}


ResolveDialog::~ResolveDialog()
{
    saveDialogSize(partConfig, "ResolveDialog");
}


// One resolve item has a line number range of linenoA:linenoA+linecountA-1
// in A and linenoB:linenoB+linecountB-1 in B. If the user has chosen version A
// for the merged file (indicated by chosenA==true), then the line number
// range in the merged file is offsetM:offsetM+linecountA-1 (accordingly for
// the other case).
class ResolveItem
{
public:
    int linenoA, linecountA;
    int linenoB, linecountB;
    int linecountTotal;
    int offsetM;
    ResolveDialog::ChooseType chosen;
};


bool ResolveDialog::parseFile(const TQString &name)
{
    int lineno1, lineno2;
    int advanced1, advanced2;
    enum { Normal, VersionA, VersionB } state;

    setCaption(i18n("CVS Resolve: %1").arg(name));

    fname = name;
  
    TQString fileContent = readFile();
    if( fileContent.isNull() )
        return false;
        
    LineSeparator separator(fileContent);

    state = Normal;
    lineno1 = lineno2 = 0;
    advanced1 = advanced2 = 0;
    do
    {
        TQString line = separator.nextLine();
        
        // reached end of file?
        if( separator.atEnd() )
            break;
            
        switch( state )
        {
            case Normal:
                {
                    // check for start of conflict block
                    // Set to look for <<<<<<< at begining of line with exaclty one
                    // space after then anything after that.
                    TQRegExp rx( "^<{7}\\s.*$" ); 
                    int separatorPos = rx.search(line);
                    if( separatorPos >= 0 )
                    {
                        state     = VersionA;
                        advanced1 = 0;
                    }
                    else
                    {
                        addToMergeAndVersionA(line, DiffView::Unchanged, lineno1);
                        addToVersionB(line, DiffView::Unchanged, lineno2);
                    }
                }
                break;
            case VersionA:
                {
                    // Set to look for ======= at begining of line which may have one
                    // or more spaces after then nothing else.
                    TQRegExp rx( "^={7}\\s*$" );
                    int separatorPos = rx.search(line);
                    if( separatorPos < 0 )    // still in version A
                    {
                        advanced1++;
                        addToMergeAndVersionA(line, DiffView::Change, lineno1);
                    }
                    else
                    {
                        state     = VersionB;
                        advanced2 = 0;
                    }
                }
                break;
            case VersionB:
                {
                    // Set to look for >>>>>>> at begining of line with exaclty one
                    // space after then anything after that.
                    TQRegExp rx( "^>{7}\\s.*$" );
                    int separatorPos = rx.search(line);
                    if( separatorPos < 0 )    // still in version B
                    {
                        advanced2++;
                        addToVersionB(line, DiffView::Change, lineno2);
                    }
                    else
                    {
                        // create an resolve item
                        ResolveItem *item = new ResolveItem;
                        item->linenoA        = lineno1-advanced1+1;
                        item->linecountA     = advanced1;
                        item->linenoB        = lineno2-advanced2+1;
                        item->linecountB     = advanced2;
                        item->offsetM        = item->linenoA-1;
                        item->chosen         = ChA;
                        item->linecountTotal = item->linecountA;
                        items.append(item);
                        
                        for (; advanced1 < advanced2; advanced1++)
                            diff1->addLine("", DiffView::Neutral);
                        for (; advanced2 < advanced1; advanced2++)
                            diff2->addLine("", DiffView::Neutral);
                            
                        state = Normal;
                    }
                }
                break;
        }
    } 
    while( !separator.atEnd() );
    
    updateNofN();

    return true; // succesful
}


void ResolveDialog::addToMergeAndVersionA(const TQString& line, 
                                          DiffView::DiffType type, int& lineNo)
{
    lineNo++;
    diff1->addLine(line, type, lineNo);
    merge->addLine(line, type, lineNo);
}


void ResolveDialog::addToVersionB(const TQString& line, DiffView::DiffType type, 
                                  int& lineNo)
{
    lineNo++;
    diff2->addLine(line, type, lineNo);
}


void ResolveDialog::saveFile(const TQString &name)
{
    TQFile f(name);
    if (!f.open(IO_WriteOnly))
    {
        KMessageBox::sorry(this,
                           i18n("Could not open file for writing."),
                           "Cervisia");
        return;
    }
    TQTextStream stream(&f);
    TQTextCodec *fcodec = DetectCodec(name);
    stream.setCodec(fcodec);
    
    TQString output;
    for( int i = 0; i < merge->count(); i++ )
       output +=merge->stringAtOffset(i);
    stream << output;
    
    f.close();
}
    

TQString ResolveDialog::readFile()
{
    TQFile f(fname);
    if( !f.open(IO_ReadOnly) )
        return TQString::null;

    TQTextStream stream(&f);
    TQTextCodec* codec = DetectCodec(fname);
    stream.setCodec(codec);
    
    return stream.read();
}


void ResolveDialog::updateNofN()
{
    TQString str;
    if (markeditem >= 0)
        str = i18n("%1 of %2").arg(markeditem+1).arg(items.count());
    else
        str = i18n("%1 conflicts").arg(items.count());
    nofnlabel->setText(str);

    backbutton->setEnabled(markeditem != -1);
    forwbutton->setEnabled(markeditem != -2 && items.count());

    bool marked = markeditem >= 0;
    abutton->setEnabled(marked);
    bbutton->setEnabled(marked);
    abbutton->setEnabled(marked);
    babutton->setEnabled(marked);
    editbutton->setEnabled(marked);
}


void ResolveDialog::updateHighlight(int newitem)
{
    if (markeditem >= 0)
    {
        ResolveItem *item = items.at(markeditem);
        for (int i = item->linenoA; i < item->linenoA+item->linecountA; ++i)
            diff1->setInverted(i, false);
        for (int i = item->linenoB; i < item->linenoB+item->linecountB; ++i)
            diff2->setInverted(i, false);
    }

    markeditem = newitem;

    if (markeditem >= 0)
    {
        ResolveItem *item = items.at(markeditem);
        for (int i = item->linenoA; i < item->linenoA+item->linecountA; ++i)
            diff1->setInverted(i, true);
        for (int i = item->linenoB; i < item->linenoB+item->linecountB; ++i)
            diff2->setInverted(i, true);
        diff1->setCenterLine(item->linenoA);
        diff2->setCenterLine(item->linenoB);
        merge->setCenterOffset(item->offsetM);
    }
    diff1->repaint();
    diff2->repaint();
    merge->repaint();
    updateNofN();
}


void ResolveDialog::updateMergedVersion(ResolveItem* item, 
                                        ResolveDialog::ChooseType chosen)
{
    // Remove old variant
    for (int i = 0; i < item->linecountTotal; ++i)
        merge->removeAtOffset(item->offsetM);

    // Insert new
    int total = 0;
    LineSeparator separator(m_contentMergedVersion);
    TQString line = separator.nextLine();
    while( !separator.atEnd() )
    {
        merge->insertAtOffset(line, DiffView::Change, item->offsetM+total);
        line = separator.nextLine();
        ++total;
    }

    // Adjust other items
    int difference = total - item->linecountTotal;
    item->chosen = chosen;
    item->linecountTotal = total;
    while ( (item = items.next()) != 0 )
        item->offsetM += difference;

    merge->repaint();
}


void ResolveDialog::backClicked()
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


void ResolveDialog::forwClicked()
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


void ResolveDialog::choose(ChooseType ch)
{
    if (markeditem < 0)
        return;

    ResolveItem *item = items.at(markeditem);

    switch (ch)
        {
        case ChA:
            m_contentMergedVersion = contentVersionA(item);
            break;
        case ChB:
            m_contentMergedVersion = contentVersionB(item);
            break;
        case ChAB:
            m_contentMergedVersion = contentVersionA(item) + contentVersionB(item);
            break;
        case ChBA:
            m_contentMergedVersion = contentVersionB(item) + contentVersionA(item);
            break;
        default:
            kdDebug(8050) << "Internal error at switch" << endl;
        }

    updateMergedVersion(item, ch);
}


void ResolveDialog::aClicked()
{
    choose(ChA);
}


void ResolveDialog::bClicked()
{
    choose(ChB);
}


void ResolveDialog::abClicked()
{
    choose(ChAB);
}


void ResolveDialog::baClicked()
{
    choose(ChBA);
}


void ResolveDialog::editClicked()
{
    if (markeditem < 0)
        return;
    
    ResolveItem *item = items.at(markeditem);
    
    TQString mergedPart;
    int total  = item->linecountTotal;
    int offset = item->offsetM;
    for( int i = 0; i < total; ++i )
        mergedPart += merge->stringAtOffset(offset+i);

    ResolveEditorDialog *dlg = new ResolveEditorDialog(partConfig, this, "edit");
    dlg->setContent(mergedPart);

    if (dlg->exec())
    {
        m_contentMergedVersion = dlg->content();
        updateMergedVersion(item, ChEdit);
    }

    delete dlg;
    diff1->repaint();
    diff2->repaint();
    merge->repaint();
}


void ResolveDialog::saveClicked()
{
    saveFile(fname);
}


void ResolveDialog::saveAsClicked()
{
    TQString filename =
        KFileDialog::getSaveFileName(0, 0, this, 0);

    if( !filename.isEmpty() && Cervisia::CheckOverwrite(filename) )
        saveFile(filename);
}


void ResolveDialog::keyPressEvent(TQKeyEvent *e)
{
    switch (e->key())
    {
        case Key_A:    aClicked();    break;
        case Key_B:    bClicked();    break;
        case Key_Left: backClicked(); break;
        case Key_Right:forwClicked(); break;
        case Key_Up:   diff1->up();   break;
        case Key_Down: diff1->down(); break;
        default:
            KDialogBase::keyPressEvent(e);
    }
}



/* This will return the A side of the diff in a TQString. */
TQString ResolveDialog::contentVersionA(const ResolveItem *item)
{
    TQString result;
    for( int i = item->linenoA; i < item->linenoA+item->linecountA; ++i )
    {
        result += diff1->stringAtLine(i);
    }
    
    return result;  
}


/* This will return the B side of the diff item in a TQString. */
TQString ResolveDialog::contentVersionB(const ResolveItem *item)
{
    TQString result;
    for( int i = item->linenoB; i < item->linenoB+item->linecountB; ++i )
    {
        result += diff2->stringAtLine(i);
    }

    return result;
}

#include "resolvedlg.moc"


// Local Variables:
// c-basic-offset: 4
// End:
