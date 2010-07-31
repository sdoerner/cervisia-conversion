/*
 * Copyright (c) 2002-2004 Christian Loose <christian.loose@kdemail.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include "repository.h"

#include <tqdir.h>
#include <tqfile.h>
#include <tqstring.h>

#include <kapplication.h>
#include <kconfig.h>
#include <kdirwatch.h>
#include <kstandarddirs.h>

#include "sshagent.h"


struct Repository::Private
{
    Private() : compressionLevel(0) {}

    TQString     configFileName;

    TQString     workingCopy;
    TQString     location;

    TQString     client;
    TQString     rsh;
    TQString     server;
    int         compressionLevel;
    bool        retrieveCvsignoreFile;

    void readConfig();
    void readGeneralConfig();
};



Repository::Repository()
    : TQObject()
    , DCOPObject("CvsRepository")
    , d(new Private)
{
    d->readGeneralConfig();

    // other cvsservice instances might change the configuration file
    // so we watch it for changes
    d->configFileName = locate("config", "cvsservicerc");
    KDirWatch* fileWatcher = new KDirWatch(this);
    connect(fileWatcher, TQT_SIGNAL(dirty(const TQString&)),
            this, TQT_SLOT(slotConfigDirty(const TQString&)));
    fileWatcher->addFile(d->configFileName);
}


Repository::Repository(const TQString& repository)
    : TQObject()
    , DCOPObject()
    , d(new Private)
{
    d->location = repository;
    d->readGeneralConfig();
    d->readConfig();

    // other cvsservice instances might change the configuration file
    // so we watch it for changes
    d->configFileName = locate("config", "cvsservicerc");
    KDirWatch* fileWatcher = new KDirWatch(this);
    connect(fileWatcher, TQT_SIGNAL(dirty(const TQString&)),
            this, TQT_SLOT(slotConfigDirty(const TQString&)));
    fileWatcher->addFile(d->configFileName);
}


Repository::~Repository()
{
    delete d;
}


TQString Repository::cvsClient() const
{
    TQString client(d->client);

    // suppress reading of the '.cvsrc' file
    client += " -f";

    // we don't need the command line option if there is no compression level set
    if( d->compressionLevel > 0 )
    {
        client += " -z" + TQString::number(d->compressionLevel) + " ";
    }

    return client;
}


TQString Repository::clientOnly() const
{
    return d->client;
}


TQString Repository::rsh() const
{
    return d->rsh;
}


TQString Repository::server() const
{
    return d->server;
}


bool Repository::setWorkingCopy(const TQString& dirName)
{
    const TQFileInfo fi(dirName);
    const TQString path = fi.absFilePath();

    // is this really a cvs-controlled directory?
    const TQFileInfo cvsDirInfo(path + "/CVS");
    if( !cvsDirInfo.exists() || !cvsDirInfo.isDir() ||
        !TQFile::exists( cvsDirInfo.filePath() + "/Entries" ) ||
        !TQFile::exists( cvsDirInfo.filePath() + "/Repository" ) ||
        !TQFile::exists( cvsDirInfo.filePath() + "/Root" ) )
        return false;

    d->workingCopy = path;
    d->location    = TQString::null;

    // determine path to the repository
    TQFile rootFile(path + "/CVS/Root");
    if( rootFile.open(IO_ReadOnly) )
    {
        TQTextStream stream(&rootFile);
        d->location = stream.readLine();
    }
    rootFile.close();

    // add identities (ssh-add) to ssh-agent
    // TODO CL make sure this is called only once
    if( d->location.contains(":ext:", false) > 0 )
    {
        SshAgent ssh;
        ssh.addSshIdentities();
    }

    TQDir::setCurrent(path);
    d->readConfig();

    return true;
}


TQString Repository::workingCopy() const
{
    return d->workingCopy;
}


TQString Repository::location() const
{
    return d->location;
}


bool Repository::retrieveCvsignoreFile() const
{
    return d->retrieveCvsignoreFile;
}


void Repository::slotConfigDirty(const TQString& fileName)
{
    if( fileName == d->configFileName )
    {
        // reread the configuration data from disk
        kapp->config()->reparseConfiguration();
        d->readConfig();
    }
}


void Repository::Private::readGeneralConfig()
{
    KConfig* config = kapp->config();

    // get path to cvs client programm
    config->setGroup("General");
    client = config->readPathEntry("CVSPath", "cvs");
}


void Repository::Private::readConfig()
{
    KConfig* config = kapp->config();

    // Sometimes the location can be unequal to the entry in the CVS/Root.
    //
    // This can happen when the checkout was done with a repository name
    // like :pserver:user@cvs.kde.org:/home/kde. When cvs then saves the
    // name into the .cvspass file, it adds the default cvs port to it like
    // this :pserver:user@cvs.kde.org:2401/home/kde. This name is then also
    // used for the configuration group.
    //
    // In order to be able to read this group, we then have to manually add
    // the port number to it.
    TQString repositoryGroup = TQString::fromLatin1("Repository-") + location;
    if( !config->hasGroup(repositoryGroup) )
    {
        // find the position of the first path separator
        const int insertPos = repositoryGroup.find('/');
        if( insertPos > 0 )
        {
            // add port to location
            // (1) :pserver:user@hostname.com:/path
            if( repositoryGroup.at(insertPos - 1) == ':' )
                repositoryGroup.insert(insertPos, "2401");
            // (2) :pserver:user@hostname.com/path
            else
                repositoryGroup.insert(insertPos, ":2401");
        }
    }

    config->setGroup(repositoryGroup);

    // should we retrieve the CVSROOT/cvsignore file from the cvs server?
    retrieveCvsignoreFile = config->readBoolEntry("RetrieveCvsignore", false);
    
    // see if there is a specific compression level set for this repository
    compressionLevel = config->readNumEntry("Compression", -1);

    // use default global compression level instead?
    if( compressionLevel < 0 )
    {
        KConfigGroupSaver cs(config, "General");
        compressionLevel = config->readNumEntry("Compression", 0);
    }

    // get remote shell client to access the remote repository
    rsh = config->readPathEntry("rsh");

    // get program to start on the server side
    server = config->readEntry("cvs_server");
}


#include "repository.moc"
