/*
 *  Copyright (c) 2003 Christian Loose <christian.loose@hamburg.de>
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


#include "watchersdlg.h"

#include <tqlayout.h>
#include <tqtable.h>
#include <kconfig.h>
#include <klineedit.h>
#include <klocale.h>

#include "misc.h"
#include "cvsservice_stub.h"
#include "progressdlg.h"


WatchersDialog::WatchersDialog(KConfig& cfg, TQWidget* tqparent, const char* name)
    : KDialogBase(tqparent, name, false, TQString(),
                  Close, ButtonCode(0), true)
    , partConfig(cfg)
{
    TQFrame* mainWidget = makeMainWidget();

    TQBoxLayout *tqlayout = new TQVBoxLayout(mainWidget, 0, spacingHint());

    table = new TQTable(mainWidget, "watchersTable");
    table->setNumCols(5);
    table->setSelectionMode(TQTable::NoSelection);
    table->setColumnMovingEnabled(false);
    table->setRowMovingEnabled(false);
    table->setReadOnly(true);
    table->setDragEnabled(false);
    table->setSorting(true);
    table->verticalHeader()->hide();
    table->setLeftMargin(0);
    
    TQHeader* header = table->horizontalHeader();
    header->setLabel(0, i18n("File"));
    header->setLabel(1, i18n("Watcher"));
    header->setLabel(2, i18n("Edit"));
    header->setLabel(3, i18n("Unedit"));
    header->setLabel(4, i18n("Commit"));
    
    tqlayout->addWidget(table, 1);

    setWFlags(TQt::WDestructiveClose | getWFlags());

    TQSize size = configDialogSize(partConfig, "WatchersDialog");
    resize(size);
}


WatchersDialog::~WatchersDialog()
{
    saveDialogSize(partConfig, "WatchersDialog");
}


bool WatchersDialog::parseWatchers(CvsService_stub* cvsService, 
                                   const TQStringList& files)
{
    setCaption(i18n("CVS Watchers"));

    DCOPRef job = cvsService->watchers(files);
    if( !cvsService->ok() )
        return false;

    ProgressDialog dlg(this, "Watchers", job, "watchers", i18n("CVS Watchers"));
    if( !dlg.execute() )
        return false;

    TQString line;
    int numRows = 0;
    while( dlg.getLine(line) )
    {
        // parse the output line        
        TQStringList list = splitLine(line);

        // ignore empty lines and unknown files
        if( list.isEmpty() || list[0] == "?" )
            continue;

        // add a new row to the table
        table->setNumRows(numRows + 1);

        table->setText(numRows, 0, list[0]);
        table->setText(numRows, 1, list[1]);

        TQCheckTableItem* item = new TQCheckTableItem(table, "");
        item->setChecked(list.tqcontains("edit"));
        table->setItem(numRows, 2, item);

        item = new TQCheckTableItem(table, "");
        item->setChecked(list.tqcontains("unedit"));
        table->setItem(numRows, 3, item);

        item = new TQCheckTableItem(table, "");
        item->setChecked(list.tqcontains("commit"));
        table->setItem(numRows, 4, item);

        ++numRows;
    }

    return true;
}
