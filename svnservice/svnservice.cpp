/*
 * Copyright (c) 2005 Christian Loose <christian.loose@kdemail.net>
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
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */

#include "svnservice.h"

#include <qintdict.h>

#include <dcopref.h>
#include <dcopclient.h>
#include <kapplication.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kprocess.h>

#include "svnjob.h"
#include "svnrepository.h"


static const char SINGLE_JOB_ID[]   = "NonConcurrentJob";
static const char REDIRECT_STDERR[] = "2>&1";


struct SvnService::Private
{
    Private() : singleSvnJob(0), lastJobId(0), repository(0) {}
    ~Private()
    {
        delete repository;
        delete singleSvnJob;
    }

    SvnJob*               singleSvnJob;   // non-concurrent svn job, like update or commit
    DCOPRef               singleJobRef;   // DCOP reference to non-concurrent cvs job
    QIntDict<SvnJob>      svnJobs;        // concurrent svn jobs, like diff or annotate
    unsigned              lastJobId;

    QCString              appId;          // cache the DCOP clients app id

    SvnRepository*        repository;

    SvnJob* createSvnJob();
    DCOPRef setupNonConcurrentJob(SvnRepository* repo = 0);

    bool hasWorkingCopy();
    bool hasRunningJob();
};


static QString JoinFileList(const QStringList& files)
{
    QString result;

    QStringList::ConstIterator it  = files.begin();
    QStringList::ConstIterator end = files.end();

    for( ; it != end; ++it )
    {
        result += KProcess::quote(*it);
        result += " ";
    }

    if( result.length() > 0 )
        result.truncate(result.length()-1);

    return result;
}


SvnService::SvnService()
    : DCOPObject("SvnService")
    , d(new Private)
{
    d->appId = kapp->dcopClient()->appId();

    // create non-concurrent subversion job
    d->singleSvnJob = new SvnJob(SINGLE_JOB_ID);
    d->singleJobRef.setRef(d->appId, d->singleSvnJob->objId());

    // create repository manager
    d->repository = new SvnRepository();

    d->svnJobs.setAutoDelete(true);
}


SvnService::~SvnService()
{
    d->svnJobs.clear();
    delete d;
}


DCOPRef SvnService::add(const QStringList& files)
{
    if( !d->hasWorkingCopy() || d->hasRunningJob() )
        return DCOPRef();

    // assemble the command line
    // svn add [FILES]
    d->singleSvnJob->clearCommand();

    *d->singleSvnJob << d->repository->svnClient() << "add"
                     << JoinFileList(files) << REDIRECT_STDERR;

    return d->setupNonConcurrentJob();
}


DCOPRef SvnService::annotate(const QString& fileName, const QString& revision)
{
    if( !d->hasWorkingCopy() )
        return DCOPRef();

    // create a svn job
    SvnJob* job = d->createSvnJob();

    // assemble the command line
    // svn annotate [FILE] [-r REVISION]
    *job << d->repository->svnClient() << "annotate" << KProcess::quote(fileName);

    if( !revision.isEmpty() )
        *job << "-r" << revision;

    // return a DCOP reference to the svn job
    return DCOPRef(d->appId, job->objId());
}


DCOPRef SvnService::commit(const QStringList& files, const QString& commitMessage,
                           bool recursive)
{
    if( !d->hasWorkingCopy() || d->hasRunningJob() )
        return DCOPRef();

    // assemble the command line
    // svn commit [-N] [-m MESSAGE] [FILES]
    d->singleSvnJob->clearCommand();

    *d->singleSvnJob << d->repository->svnClient() << "commit";

    if( !recursive )
        *d->singleSvnJob << "-N";

    *d->singleSvnJob << "-m" << KProcess::quote(commitMessage)
                     << JoinFileList(files) << REDIRECT_STDERR;

    return d->setupNonConcurrentJob();
}


DCOPRef SvnService::diff(const QString& fileName,
                         const QString& revisionA,
                         const QString& revisionB,
                         const QStringList& options)
{
    if( !d->hasWorkingCopy() )
        return DCOPRef();

    // create a svn job
    SvnJob* job = d->createSvnJob();

    // assemble the command line
    // svn diff [DIFFOPTIONS] [FORMAT] [-r REVA] {-r REVB] [FILE]
    *job << d->repository->svnClient() << "diff";

    QString revision;
    if( !revisionA.isEmpty() && !revisionB.isEmpty() )
        revision = revisionA + ':' + revisionB;
    else if( !revisionA.isEmpty() )
        revision = revisionA;
    else if( !revisionB.isEmpty() )
        revision = revisionB;

    if( !revision.isEmpty() )
        *job << "-r" << KProcess::quote(revision);

    *job << KProcess::quote(fileName);

    // return a DCOP reference to the svn job
    return DCOPRef(d->appId, job->objId());
}


DCOPRef SvnService::log(const QString& fileName)
{
    if( !d->hasWorkingCopy() )
        return DCOPRef();

    // create a svn job
    SvnJob* job = d->createSvnJob();

    // assemble the command line
    // svn log [FILE]
    *job << d->repository->svnClient() << "log" << KProcess::quote(fileName);

    // return a DCOP reference to the svn job
    return DCOPRef(d->appId, job->objId());
}


DCOPRef SvnService::remove(const QStringList& files)
{
    if( !d->hasWorkingCopy() || d->hasRunningJob() )
        return DCOPRef();

    // assemble the command line
    // svn delete [FILES]
    d->singleSvnJob->clearCommand();

    *d->singleSvnJob << d->repository->svnClient() << "delete"
                     << JoinFileList(files) << REDIRECT_STDERR;

    return d->setupNonConcurrentJob();
}


DCOPRef SvnService::revert(const QStringList& files, bool recursive)
{
    if( !d->hasWorkingCopy() || d->hasRunningJob() )
        return DCOPRef();

    // assemble the command line
    // svn revert [-R] [FILES]
    d->singleSvnJob->clearCommand();

    *d->singleSvnJob << d->repository->svnClient() << "revert";

    if( recursive )
        *d->singleSvnJob << "-R";

    *d->singleSvnJob << JoinFileList(files) << REDIRECT_STDERR;

    return d->setupNonConcurrentJob();
}


DCOPRef SvnService::simulateUpdate(const QStringList& files, bool recursive)
{
    if( !d->hasWorkingCopy() || d->hasRunningJob() )
        return DCOPRef();

    // assemble the command line
    // svn status [-N] [FILES]
    d->singleSvnJob->clearCommand();

    *d->singleSvnJob << d->repository->svnClient() << "status" << "-u";

    if( !recursive )
        *d->singleSvnJob << "-N";

    *d->singleSvnJob << JoinFileList(files) << REDIRECT_STDERR;

    return d->setupNonConcurrentJob();
}


DCOPRef SvnService::update(const QStringList& files, bool recursive)
{
    if( !d->hasWorkingCopy() || d->hasRunningJob() )
        return DCOPRef();

    // assemble the command line
    // svn update [-N] [FILES]
    d->singleSvnJob->clearCommand();

    *d->singleSvnJob << d->repository->svnClient() << "update";

    if( !recursive )
        *d->singleSvnJob << "-N";

    *d->singleSvnJob << JoinFileList(files) << REDIRECT_STDERR;

    return d->setupNonConcurrentJob();
}


void SvnService::quit()
{
    kapp->quit();
}


SvnJob* SvnService::Private::createSvnJob()
{
    ++lastJobId;

    // create a new subversion job
    SvnJob* job = new SvnJob(lastJobId);
    svnJobs.insert(lastJobId, job);

    job->setDirectory(repository->workingCopy());

    return job;
}


DCOPRef SvnService::Private::setupNonConcurrentJob(SvnRepository* repo)
{
    // no explicit repository provided?
    if( !repo )
        repo = repository;

    singleSvnJob->setDirectory(repo->workingCopy());

    return singleJobRef;
}


bool SvnService::Private::hasWorkingCopy()
{
    if( repository->workingCopy().isEmpty() )
    {
        KMessageBox::sorry(0, i18n("You have to set a local working copy "
                                   "directory before you can use this function!"));
        return false;
    }

    return true;
}


bool SvnService::Private::hasRunningJob()
{
    bool result = singleSvnJob->isRunning();

    if( result )
        KMessageBox::sorry(0, i18n("There is already a job running"));

    return result;
}
