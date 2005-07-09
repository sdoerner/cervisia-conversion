/* 
 *  Copyright (C) 2004 Christian Loose <christian.loose@kdemail.net>
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "globalignorelist.h"
using namespace Cervisia;

#include <qdir.h>
#include <kdebug.h>
#include <ktempfile.h>
#include <stdlib.h> // for getenv()

#include "cvsservice_stub.h"
#include "progressdlg.h"
#include "stringmatcher.h"


struct Data
{
    static StringMatcher m_stringMatcher;
    static bool          m_isInitialized;
};

StringMatcher Data::m_stringMatcher;
bool          Data::m_isInitialized = false;


GlobalIgnoreList::GlobalIgnoreList(IgnoreFilterBase* nextFilter)
    : IgnoreFilterBase(nextFilter)
{
    if( !Data().m_isInitialized )
        setup();
}

//TODO: restore functionality
void GlobalIgnoreList::retrieveServerIgnoreList(CvsService_stub* cvsService,
                                                const QString& repository)
{
//     KTempFile tmpFile;
//     tmpFile.setAutoDelete(true);
//        
//     // clear old ignore list
//     m_stringMatcher.clear();
//     
//     // now set it up again
//     setup();
//     
//     DCOPRef ref = cvsService->downloadCvsIgnoreFile(repository, 
//                                                     tmpFile.name());
//       
//     ProgressDialog dlg(0, "Edit", ref, "checkout", "CVS Edit");
//     if( !dlg.execute() )
//         return;
//     
//     addEntriesFromFile(tmpFile.name());
}


bool GlobalIgnoreList::doMatches(const QString& fileName) const
{
    return Data().m_stringMatcher.match(fileName);
}


void GlobalIgnoreList::addEntry(const QString& entry)
{
    if (entry != QChar('!'))
    {
        Data().m_stringMatcher.add(entry);
    }
    else
    {
        Data().m_stringMatcher.clear();

        // Bug #89215:
        // Make sure '.' and '..' are always in the ignore list, so
        // UpdateDirItem::maybeScanDir() doesn't loop endlessly.
        addEntriesFromString(QString::fromLatin1(". .."));
    }
}


void GlobalIgnoreList::setup()
{
    static const char ignorestr[] = ". .. core RCSLOG tags TAGS RCS SCCS .make.state\
.nse_depinfo #* .#* cvslog.* ,* CVS CVS.adm .del-* *.a *.olb *.o *.obj\
*.so *.Z *~ *.old *.elc *.ln *.bak *.BAK *.orig *.rej *.exe _$* *$";
    
    addEntriesFromString(QString::fromLatin1(ignorestr));
    addEntriesFromString(QString::fromLocal8Bit(::getenv("CVSIGNORE")));
    addEntriesFromFile(QDir::homeDirPath() + "/.cvsignore");  
    
    Data().m_isInitialized = true;
}
