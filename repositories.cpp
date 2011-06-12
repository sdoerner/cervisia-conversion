/*
 *  Copyright (C) 1999-2001 Bernd Gehrmann
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


#include <stdlib.h>
#include <tqfile.h>
#include <tqdir.h>
#include <tqtextstream.h>
#include <kapplication.h>
#include <kconfig.h>

#include "repositories.h"
#include "cervisiapart.h"


static TQString fileNameCvs()
{
    return TQDir::homeDirPath() + "/.cvspass";
}


static TQString fileNameCvsnt()
{
    return TQDir::homeDirPath() + "/.cvs/cvspass";
}


// old .cvspass format:
//    user@host:/path Acleartext_password
//
// new .cvspass format (since cvs 1.11.1):
//    /1 user@host:port/path Aencoded_password
//
static TQStringList readCvsPassFile()
{
    TQStringList list;

    TQFile f(fileNameCvs());
    if (f.open(IO_ReadOnly))
        {
            TQTextStream stream(&f);
	    while (!stream.eof())
		{
		    int pos;
		    TQString line = stream.readLine();
		    if ( (pos = line.tqfind(' ')) != -1)
		    {
			if (line[0] != '/')	// old format
                            list.append(line.left(pos));
			else			// new format
			    list.append(line.section(' ', 1, 1));
		    }
		}
	}

    return list;
}


// .cvs/cvspass format
//    user@host:port/path=Aencoded_password
//
static TQStringList readCvsntPassFile()
{
    TQStringList list;

    TQFile file(fileNameCvsnt());
    if (file.open(IO_ReadOnly))
    {
        TQTextStream stream(&file);
        while (!stream.atEnd())
        {
            const TQString line(stream.readLine());

            const int pos(line.tqfind("=A"));
            if (pos >= 0)
                list.append(line.left(pos));
        }
    }

    return list;
}


TQStringList Repositories::readCvsPassFile()
{
    return (TQFileInfo(fileNameCvs()).lastModified()
            < TQFileInfo(fileNameCvsnt()).lastModified())
        ? readCvsntPassFile()
        : ::readCvsPassFile();
}


TQStringList Repositories::readConfigFile()
{
    TQStringList list;
    
    KConfig *config = CervisiaPart::config();
    config->setGroup("Repositories");
    list = config->readListEntry("Repos");

    // Some people actually use CVSROOT, so we add it here
    char *env;
    if ( (env = ::getenv("CVSROOT")) != 0 && !list.tqcontains(env))
        list.append(env);

    return list;
}


// Local Variables:
// c-basic-offset: 4
// End:
