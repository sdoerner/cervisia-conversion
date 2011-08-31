/*
 *  Copyright (C) 1999-2002 Bernd Gehrmann
 *                          bernd@mail.berlios.de
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


#include "misc.h"

#include <ctype.h>
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>
#include <tqfile.h>
#include <tqfileinfo.h>
#include <tqregexp.h>
#include <tqstringlist.h>
#include <kconfig.h>
#include <kemailsettings.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kprocess.h>
#include <ktempfile.h>
#include <kuser.h>
#include <kdebug.h>

#include "cvsservice_stub.h"
#include "progressdlg.h"

// These regular expression parts aren't useful to check the validity of the
// CVSROOT specification. They are just used to extract the different parts of it.
static const TQString userNameRegExp("([a-z0-9_][a-z0-9_-.]*)?");
static const TQString passwordRegExp("(:[^@]+)?");
static const TQString hostNameRegExp("([^:/@]+)");
static const TQString portRegExp("(:(\\d*))?");
static const TQString pathRegExp("(/.*)");


static int FindWhiteSpace(const TQString& str, int index)
{
    const int length = str.length();

    if( index < 0 )
        index += length;

    if( index < 0 || index >= length )
        return -1;

    const TQChar* const startPos = str.tqunicode();
    const TQChar* const endPos   = startPos + length;

    const TQChar* pos = startPos + index;
    while( pos < endPos && !pos->isSpace() )
        ++pos;

    const int foundIndex = pos - startPos;
    return (foundIndex < length ? foundIndex : -1);
}


static const TQStringList FetchBranchesAndTags(const TQString& searchedType,
                                              CvsService_stub* cvsService,
                                              TQWidget* parent)
{
    TQStringList branchOrTagList;

    DCOPRef job = cvsService->status(TQStringList(), true, true);
    if( !cvsService->ok() )
        return branchOrTagList;

    ProgressDialog dlg(parent, "Status", job, TQString(), i18n("CVS Status"));

    if( dlg.execute() )
    {
        TQString line;
        while( dlg.getLine(line) )
        {
            int wsPos, bracketPos, colonPos;

            if( line.isEmpty() || line[0] != '\t' )
                continue;
            if( (wsPos = FindWhiteSpace(line, 2)) < 0 )
                continue;
            if( (bracketPos = line.find('(', wsPos + 1)) < 0 )
                continue;
            if( (colonPos = line.find(':', bracketPos + 1)) < 0 )
                continue;

            const TQString tag  = line.mid(1, wsPos - 1);
            const TQString type = line.mid(bracketPos + 1, colonPos - bracketPos - 1);
            if( type == searchedType && !branchOrTagList.contains(tag) )
                branchOrTagList.push_back(tag);
        }

        branchOrTagList.sort();
    }

    return branchOrTagList;
}


bool Cervisia::IsValidTag(const TQString& tag)
{
    static const TQString prohibitedChars("$,.:;@");

    if( !isalpha(tag[0].latin1()) )
        return false;

    for( uint i = 1; i < tag.length(); ++i )
    {
        if( !isgraph(tag[i].latin1()) || prohibitedChars.contains(tag[i]) )
                return false;
    }

    return true;
}


TQString Cervisia::UserName()
{
    // 1. Try to retrieve the information from the control center settings
    KEMailSettings settings;
    TQString name  = settings.getSetting(KEMailSettings::RealName);
    TQString email = settings.getSetting(KEMailSettings::EmailAddress);

    if( name.isEmpty() || email.isEmpty() )
    {
        // 2. Try to retrieve the information from the system
        struct passwd* pw = getpwuid(getuid());
        if( !pw )
            return TQString();

        char hostname[512];
        hostname[0] = '\0';

        if( !gethostname(hostname, sizeof(hostname)) )
            hostname[sizeof(hostname)-1] = '0';

        name  = TQString::fromLocal8Bit(pw->pw_gecos);
        email = TQString::fromLocal8Bit(pw->pw_name) + "@" +
                TQString::fromLocal8Bit(hostname);
    }

    TQString result = name;
    result += "  <";
    result += email;
    result += ">";

    return result;
}


TQString Cervisia::NormalizeRepository(const TQString& repository)
{
    // only :pserver: repositories
    if( !repository.startsWith(":pserver:") )
        return repository;

    TQRegExp rx(":pserver:(" + userNameRegExp + passwordRegExp + "@)?" +
               hostNameRegExp + portRegExp + pathRegExp);

    // extract username, hostname, port and path from CVSROOT
    TQString userName, hostName, port, path;
    if( rx.search(repository) != -1 )
    {
        userName = rx.cap(2);
        hostName = rx.cap(4);
        port     = rx.cap(6);
        path     = rx.cap(7);

        kdDebug() << "NormalizeRepository(): username=" << userName << endl;
        kdDebug() << "NormalizeRepository(): hostname=" << hostName << endl;
        kdDebug() << "NormalizeRepository(): port    =" << port << endl;
        kdDebug() << "NormalizeRepository(): path    =" << path << endl;

        if( port.isEmpty() )
            port = "2401";

        if( userName.isEmpty() )
            userName = KUser().loginName();

        TQString canonicalForm = ":pserver:" + userName + "@" + hostName +
                                ":" + port + path;

        kdDebug() << "NormalizeRepository(): canonicalForm=" << canonicalForm
                  << endl;
        return canonicalForm;
    }
    else
        return repository;
}


bool Cervisia::CheckOverwrite(const TQString& fileName, TQWidget* parent)
{
    bool result = true;

    TQFileInfo fi(fileName);

    // does the file already exist?
    if( fi.exists() )
    {
        result = (KMessageBox::warningContinueCancel(parent,
                  i18n("A file named \"%1\" already exists. Are you sure you want to overwrite it?").tqarg(fileName),
                  i18n("Overwrite File?"),
                  KGuiItem(i18n("&Overwrite"), "filesave", i18n("Overwrite the file"))) == KMessageBox::Continue);
    }

    return result;
}


TQString joinLine(const TQStringList &list)
{
    TQString line;
    for ( TQStringList::ConstIterator it = list.begin();
          it != list.end(); ++it )
    {
        line += KShellProcess::quote(*it);
        line += " ";
    }

    if (line.length() > 0)
        line.truncate(line.length()-1);

    return line;
}


// Should be replaceable by TQStringList::split
TQStringList splitLine(TQString line, char delim)
{
    int pos;
    TQStringList list;

    line = line.simplifyWhiteSpace();
    while ((pos = line.find(delim)) != -1)
    {
        list.append(line.left(pos));
        line = line.mid(pos+1, line.length()-pos-1);
    }
    if (!line.isEmpty())
        list.append(line);
    return list;
}


const TQStringList fetchBranches(CvsService_stub* cvsService, TQWidget* parent)
{
    return FetchBranchesAndTags(TQString::tqfromLatin1("branch"), cvsService,
                                parent);
}


const TQStringList fetchTags(CvsService_stub* cvsService, TQWidget* parent)
{
    return FetchBranchesAndTags(TQString::tqfromLatin1("revision"), cvsService,
                                parent);
}


static TQStringList *tempFiles = 0;

void cleanupTempFiles()
{
    if (tempFiles)
    {
        TQStringList::Iterator it;
        for (it = tempFiles->begin(); it != tempFiles->end(); ++it)
            TQFile::remove(*it);
        delete tempFiles;
    }
}


TQString tempFileName(const TQString& suffix)
{
    if (!tempFiles)
        tempFiles = new TQStringList;

    KTempFile f(TQString(), suffix);
    tempFiles->append(f.name());
    return f.name();
}


int compareRevisions(const TQString& rev1, const TQString& rev2)
{
    const int length1(rev1.length());
    const int length2(rev2.length());

    // compare all parts of the revision

    int startPos1(0);
    int startPos2(0);
    while (startPos1 < length1 && startPos2 < length2)
    {
        int pos1(rev1.find('.', startPos1));
        if (pos1 < 0)
            pos1 = length1;
        const int partLength1(pos1 - startPos1);

        int pos2(rev2.find('.', startPos2));
        if (pos2 < 0)
            pos2 = length2;
        const int partLength2(pos2 - startPos2);

        // if the number of digits in both parts is not equal we are ready
        if (const int comp = ::compare(partLength1, partLength2))
            return comp;

        // if the parts are not equal we are ready
        if (const int comp = ::compare(rev1.mid(startPos1, partLength1),
                                       rev2.mid(startPos2, partLength2)))
            return comp;

        // continue with next part
        startPos1 = pos1 + 1;
        startPos2 = pos2 + 1;
    }

    // rev1 has more parts than rev2: rev2 < rev1
    if (startPos1 < length1)
        return 1;
    // rev2 has more parts than rev1: rev1 < rev2
    else if (startPos2 < length2)
        return -1;
    // all parts of rev1 and rev2 were compared (the number of parts is equal): rev1 == rev2
    else
        return 0;
}


// Local Variables:
// c-basic-offset: 4
// End:
