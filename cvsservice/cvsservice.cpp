/*
 * Copyright (c) 2002-2003 Christian Loose <christian.loose@hamburg.de>
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

#include "cvsservice.h"

#include <memory>

#include <qintdict.h>
#include <qstring.h>

#include <dcopref.h>
#include <dcopclient.h>
#include <kapplication.h>
#include <kconfig.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kprocess.h>

#include "cvsjob.h"
#include "cvsloginjob.h"
#include "cvsserviceutils.h"
#include "repository.h"
#include "sshagent.h"


static const char SINGLE_JOB_ID[]   = "NonConcurrentJob";
static const char REDIRECT_STDERR[] = "2>&1";


struct CvsService::Private
{
    Private() : singleCvsJob(0), lastJobId(0), repository(0) {}
    ~Private()
    {
        delete repository;
        delete singleCvsJob;
    }

    CvsJob*             singleCvsJob;   // non-concurrent cvs job, like update or commit
    DCOPRef             singleJobRef;   // DCOP reference to non-concurrent cvs job
    QIntDict<CvsJob>    cvsJobs;        // concurrent cvs jobs, like diff or annotate
    unsigned            lastJobId;

    QCString            appId;          // cache the DCOP clients app id

    Repository*         repository;

    CvsJob* createCvsJob();
    DCOPRef setupNonConcurrentJob();

    bool hasWorkingCopy();
    bool hasRunningJob();
};


CvsService::CvsService()
    : DCOPObject("CvsService")
    , d(new Private)
{
    d->appId = kapp->dcopClient()->appId();

    // create non-concurrent cvs job
    d->singleCvsJob = new CvsJob(SINGLE_JOB_ID);
    d->singleJobRef.setRef(d->appId, d->singleCvsJob->objId());

    // create repository manager
    d->repository = new Repository();

    d->cvsJobs.setAutoDelete(true);

    KConfig* config = kapp->config();
    KConfigGroupSaver cs(config, "General");
    if( config->readBoolEntry("UseSshAgent", false) )
    {
        // use the existing or start a new ssh-agent
        SshAgent ssh;
        // TODO CL do we need the return value?
        //bool res = ssh.querySshAgent();
        ssh.querySshAgent();
    }
}


CvsService::~CvsService()
{
    // kill the ssh-agent (when we started it)
    SshAgent ssh;
    ssh.killSshAgent();

    d->cvsJobs.clear();
    delete d;
}


DCOPRef CvsService::add(const QStringList& files, bool isBinary)
{
    if( !d->hasWorkingCopy() || d->hasRunningJob() )
        return DCOPRef();

    // assemble the command line
    // cvs add [-kb] [FILES]
    d->singleCvsJob->clearCvsCommand();

    *d->singleCvsJob << d->repository->cvsClient() << "add";

    if( isBinary )
        *d->singleCvsJob << "-kb";

    *d->singleCvsJob << CvsServiceUtils::joinFileList(files) << REDIRECT_STDERR;

    return d->setupNonConcurrentJob();
}


DCOPRef CvsService::annotate(const QString& fileName, const QString& revision)
{
    if( !d->hasWorkingCopy() )
        return DCOPRef();

    // create a cvs job
    CvsJob* job = d->createCvsJob();

    // assemble the command line
    // (cvs log [FILE] && cvs annotate [-r rev] [FILE])
    QString quotedName = KProcess::quote(fileName);
    QString cvsClient  = d->repository->cvsClient();

    *job << "(" << cvsClient << "log" << quotedName << "&&"
         << cvsClient << "annotate";

    if( !revision.isEmpty() )
        *job << "-r" << revision;

    // *Hack*
    // because the string "Annotations for blabla" is
    // printed to stderr even with option -Q.
    *job << quotedName << ")" << REDIRECT_STDERR;

    // return a DCOP reference to the cvs job
    return DCOPRef(d->appId, job->objId());
}


DCOPRef CvsService::checkout(const QString& workingDir, const QString& repository,
                             const QString& module, const QString& tag, bool pruneDirs)
{
    if( d->hasRunningJob() )
        return DCOPRef();

    std::auto_ptr<Repository> repo(new Repository(repository));

    // assemble the command line
    // cd [DIRECTORY] && cvs -d [REPOSITORY] checkout [-r tag] [-P] [MODULE]
    d->singleCvsJob->clearCvsCommand();

    *d->singleCvsJob << "cd" << workingDir << "&&"
                     << repo->cvsClient()
                     << "-d" << repository
                     << "checkout";

    if( !tag.isEmpty() )
        *d->singleCvsJob << "-r" << tag;

    if( pruneDirs )
        *d->singleCvsJob << "-P";

    *d->singleCvsJob << module;

    return d->setupNonConcurrentJob();
}


DCOPRef CvsService::commit(const QStringList& files, const QString& commitMessage,
                           bool recursive)
{
    if( !d->hasWorkingCopy() || d->hasRunningJob() )
        return DCOPRef();

    // assemble the command line
    // cvs commit [-l] [-m MESSAGE] [FILES]
    d->singleCvsJob->clearCvsCommand();

    *d->singleCvsJob << d->repository->cvsClient() << "commit";

    if( !recursive )
        *d->singleCvsJob << "-l";

    *d->singleCvsJob << "-m" << KProcess::quote(commitMessage)
                     << CvsServiceUtils::joinFileList(files) << REDIRECT_STDERR;

    return d->setupNonConcurrentJob();
}


DCOPRef CvsService::createTag(const QStringList& files, const QString& tag,
                              bool branch, bool force)
{
    if( !d->hasWorkingCopy() || d->hasRunningJob() )
        return DCOPRef();

    // assemble the command line
    // cvs tag [-b] [-F] [TAG] [FILES]
    d->singleCvsJob->clearCvsCommand();

    *d->singleCvsJob << d->repository->cvsClient() << "tag";

    if( branch )
        *d->singleCvsJob << "-b";

    if( force )
        *d->singleCvsJob << "-F";

    *d->singleCvsJob << KProcess::quote(tag)
                     << CvsServiceUtils::joinFileList(files);

    return d->setupNonConcurrentJob();
}


DCOPRef CvsService::deleteTag(const QStringList& files, const QString& tag,
                              bool branch, bool force)
{
    if( !d->hasWorkingCopy() || d->hasRunningJob() )
        return DCOPRef();

    // assemble the command line
    // cvs tag -d [-b] [-F] [TAG] [FILES]
    d->singleCvsJob->clearCvsCommand();

    *d->singleCvsJob << d->repository->cvsClient() << "tag" << "-d";

    if( branch )
        *d->singleCvsJob << "-b";

    if( force )
        *d->singleCvsJob << "-F";

    *d->singleCvsJob << KProcess::quote(tag)
                     << CvsServiceUtils::joinFileList(files);

    return d->setupNonConcurrentJob();
}


DCOPRef CvsService::downloadRevision(const QString& fileName,
                                     const QString& revision,
                                     const QString& outputFile)
{
    if( !d->hasWorkingCopy() )
        return DCOPRef();

    // create a cvs job
    CvsJob* job = d->createCvsJob();

    // assemble the command line
    // cvs update -p -r [REV] [FILE] > [OUTPUTFILE]
    *job << d->repository->cvsClient() << "update -p";

    if( !revision.isEmpty() )
        *job << "-r" << KProcess::quote(revision);

    *job << KProcess::quote(fileName) << ">" << KProcess::quote(outputFile);

    // return a DCOP reference to the cvs job
    return DCOPRef(d->appId, job->objId());
}


DCOPRef CvsService::downloadRevision(const QString& fileName,
                                     const QString& revA,
                                     const QString& outputFileA,
                                     const QString& revB,
                                     const QString& outputFileB)
{
    if( !d->hasWorkingCopy() )
        return DCOPRef();

    // create a cvs job
    CvsJob* job = d->createCvsJob();

    // assemble the command line
    // cvs update -p -r [REVA] [FILE] > [OUTPUTFILEA] ;
    // cvs update -p -r [REVB] [FILE] > [OUTPUTFILEB]
    *job << d->repository->cvsClient() << "update -p"
         << "-r" << KProcess::quote(revA)
         << KProcess::quote(fileName) << ">" << KProcess::quote(outputFileA)
         << ";" << d->repository->cvsClient() << "update -p"
         << "-r" << KProcess::quote(revB)
         << KProcess::quote(fileName) << ">" << KProcess::quote(outputFileB);

    // return a DCOP reference to the cvs job
    return DCOPRef(d->appId, job->objId());
}


DCOPRef CvsService::diff(const QString& fileName, const QString& revA,
                         const QString& revB, const QString& diffOptions,
                         unsigned contextLines)
{
    if( !d->hasWorkingCopy() )
        return DCOPRef();

    // create a cvs job
    CvsJob* job = d->createCvsJob();

    // assemble the command line
    // cvs diff [DIFFOPTIONS] -U CONTEXTLINES [-r REVA] {-r REVB] [FILE]
    *job << d->repository->cvsClient() << "diff" << diffOptions
         << "-U" << QString::number(contextLines);

    if( !revA.isEmpty() )
        *job << "-r" << KProcess::quote(revA);

    if( !revB.isEmpty() )
        *job << "-r" << KProcess::quote(revB);

    *job << KProcess::quote(fileName);

    // return a DCOP reference to the cvs job
    return DCOPRef(d->appId, job->objId());
}


DCOPRef CvsService::edit(const QStringList& files)
{
    if( !d->hasWorkingCopy() || d->hasRunningJob() )
        return DCOPRef();

    // assemble the command line
    // cvs edit [FILES]
    d->singleCvsJob->clearCvsCommand();

    *d->singleCvsJob << d->repository->cvsClient() << "edit"
                     << CvsServiceUtils::joinFileList(files);

    return d->setupNonConcurrentJob();
}


DCOPRef CvsService::editors(const QStringList& files)
{
    if( !d->hasWorkingCopy() || d->hasRunningJob() )
        return DCOPRef();

    // assemble the command line
    // cvs editors [FILES]
    d->singleCvsJob->clearCvsCommand();

    *d->singleCvsJob << d->repository->cvsClient() << "editors"
                     << CvsServiceUtils::joinFileList(files);

    return d->setupNonConcurrentJob();
}


DCOPRef CvsService::history()
{
    if( !d->hasWorkingCopy() )
        return DCOPRef();

    // create a cvs job
    CvsJob* job = d->createCvsJob();

    // assemble the command line
    // cvs history -e -a
    *job << d->repository->cvsClient() << "history -e -a";

    // return a DCOP reference to the cvs job
    return DCOPRef(d->appId, job->objId());
}


DCOPRef CvsService::import(const QString& workingDir, const QString& repository,
                           const QString& module, const QString& ignoreList,
                           const QString& comment, const QString& vendorTag,
                           const QString& releaseTag, bool importAsBinary)
{
    if( d->hasRunningJob() )
        return DCOPRef();

    std::auto_ptr<Repository> repo(new Repository(repository));

    // assemble the command line
    d->singleCvsJob->clearCvsCommand();

    *d->singleCvsJob << "cd" << workingDir << "&&"
                     << repo->cvsClient()
                     << "-d" << repository
                     << "import";

    if( importAsBinary )
        *d->singleCvsJob << "-kb";

    const QString ignore = ignoreList.stripWhiteSpace();
    if( !ignore.isEmpty() )
        *d->singleCvsJob << "-I" << KProcess::quote(ignore);

    QString logMessage = comment.stripWhiteSpace();
    logMessage.prepend("\"");
    logMessage.append("\"");
    *d->singleCvsJob << "-m" << logMessage;

    *d->singleCvsJob << module << vendorTag << releaseTag;

    return d->setupNonConcurrentJob();
}


DCOPRef CvsService::lock(const QStringList& files)
{
    if( !d->hasWorkingCopy() || d->hasRunningJob() )
        return DCOPRef();

    // assemble the command line
    // cvs admin -l [FILES]
    d->singleCvsJob->clearCvsCommand();

    *d->singleCvsJob << d->repository->cvsClient() << "admin -l"
                     << CvsServiceUtils::joinFileList(files);

    return d->setupNonConcurrentJob();
}


DCOPRef CvsService::log(const QString& fileName)
{
    if( !d->hasWorkingCopy() )
        return DCOPRef();

    // create a cvs job
    CvsJob* job = d->createCvsJob();

    // assemble the command line
    // cvs log [FILE]
    *job << d->repository->cvsClient() << "log" << KProcess::quote(fileName);

    // return a DCOP reference to the cvs job
    return DCOPRef(d->appId, job->objId());
}


DCOPRef CvsService::login(const QString& repository)
{
    if( repository.isEmpty() )
        return DCOPRef();

    std::auto_ptr<Repository> repo(new Repository(repository));

    // create a cvs job
    ++(d->lastJobId);

    // TODO: we need to delete this job object
    CvsLoginJob* job = new CvsLoginJob(d->lastJobId);

    // TODO: CVS_RSH and CVS_SERVER don't work ATM
//    job->setRSH(repo->rsh());
//    job->setServer(repo->server());

    // assemble the command line
    // cvs -d [REPOSITORY] login
    job->setCvsClient(repo->clientOnly().local8Bit());
    job->setRepository(repository.local8Bit());

    // return a DCOP reference to the cvs job
    return DCOPRef(d->appId, job->objId());
}


DCOPRef CvsService::logout(const QString& repository)
{
    if( repository.isEmpty() )
        return DCOPRef();

    std::auto_ptr<Repository> repo(new Repository(repository));

    // create a cvs job
    ++(d->lastJobId);

    CvsJob* job = new CvsJob(d->lastJobId);
    d->cvsJobs.insert(d->lastJobId, job);

    job->setRSH(repo->rsh());
    job->setServer(repo->server());
    job->setDirectory(repo->workingCopy());

    // assemble the command line
    // cvs -d [REPOSITORY] logout
    *job << repo->cvsClient() << "-d" << repository << "logout";

    // return a DCOP reference to the cvs job
    return DCOPRef(d->appId, job->objId());
}


DCOPRef CvsService::makePatch()
{
    if( !d->hasWorkingCopy() )
        return DCOPRef();

    // create a cvs job
    CvsJob* job = d->createCvsJob();

    // assemble the command line
    // cvs diff -uR 2>/dev/null
    *job << d->repository->cvsClient() << "diff" << "-uR"
         << "2>/dev/null";

    // return a DCOP reference to the cvs job
    return DCOPRef(d->appId, job->objId());
}


DCOPRef CvsService::moduleList(const QString& repository)
{
    std::auto_ptr<Repository> repo(new Repository(repository));

    // create a cvs job
    ++(d->lastJobId);

    CvsJob* job = new CvsJob(d->lastJobId);
    d->cvsJobs.insert(d->lastJobId, job);

    job->setRSH(repo->rsh());
    job->setServer(repo->server());
    job->setDirectory(repo->workingCopy());

    // assemble the command line
    // cvs -d [REPOSITORY] checkout -c
    *job << repo->cvsClient() << "-d" << repository << "checkout -c";

    // return a DCOP reference to the cvs job
    return DCOPRef(d->appId, job->objId());
}


DCOPRef CvsService::remove(const QStringList& files, bool recursive)
{
    if( !d->hasWorkingCopy() || d->hasRunningJob() )
        return DCOPRef();

    // assemble the command line
    // cvs remove -f [-l] [FILES]
    d->singleCvsJob->clearCvsCommand();

    *d->singleCvsJob << d->repository->cvsClient() << "remove -f";

    if( !recursive )
        *d->singleCvsJob << "-l";

    *d->singleCvsJob << CvsServiceUtils::joinFileList(files) << REDIRECT_STDERR;

    return d->setupNonConcurrentJob();
}


DCOPRef CvsService::simulateUpdate(const QStringList& files, bool recursive,
                                   bool createDirs, bool pruneDirs)
{
    if( !d->hasWorkingCopy() || d->hasRunningJob() )
        return DCOPRef();

    // assemble the command line
    // cvs -n update [-l] [-d] [-P] [FILES]
    d->singleCvsJob->clearCvsCommand();

    *d->singleCvsJob << d->repository->cvsClient() << "-n -q update";

    if( !recursive )
        *d->singleCvsJob << "-l";

    if( createDirs )
        *d->singleCvsJob << "-d";

    if( pruneDirs )
        *d->singleCvsJob << "-P";

    *d->singleCvsJob << CvsServiceUtils::joinFileList(files) << REDIRECT_STDERR;

    return d->setupNonConcurrentJob();
}


DCOPRef CvsService::status(const QStringList& files, bool recursive, bool tagInfo)
{
    if( !d->hasWorkingCopy() )
        return DCOPRef();

    // create a cvs job
    CvsJob* job = d->createCvsJob();

    // assemble the command line
    // cvs status [-l] [-v] [FILES]
    *job << d->repository->cvsClient() << "status";

    if( !recursive )
        *job << "-l";

    if( tagInfo )
        *job << "-v";

    *job << CvsServiceUtils::joinFileList(files);

    // return a DCOP reference to the cvs job
    return DCOPRef(d->appId, job->objId());
}


DCOPRef CvsService::unedit(const QStringList& files)
{
    if( !d->hasWorkingCopy() || d->hasRunningJob() )
        return DCOPRef();

    // assemble the command line
    // echo y | cvs unedit [FILES]
    d->singleCvsJob->clearCvsCommand();

    *d->singleCvsJob << "echo y |"
                     << d->repository->cvsClient() << "unedit"
                     << CvsServiceUtils::joinFileList(files);

    return d->setupNonConcurrentJob();
}


DCOPRef CvsService::unlock(const QStringList& files)
{
    if( !d->hasWorkingCopy() || d->hasRunningJob() )
        return DCOPRef();

    // assemble the command line
    // cvs admin -u [FILES]
    d->singleCvsJob->clearCvsCommand();

    *d->singleCvsJob << d->repository->cvsClient() << "admin -u"
                     << CvsServiceUtils::joinFileList(files);

    return d->setupNonConcurrentJob();
}


DCOPRef CvsService::update(const QStringList& files, bool recursive,
                           bool createDirs, bool pruneDirs, const QString& extraOpt)
{
    if( !d->hasWorkingCopy() || d->hasRunningJob() )
        return DCOPRef();

    // assemble the command line
    // cvs update [-l] [-d] [-P] [EXTRAOPTIONS] [FILES]
    d->singleCvsJob->clearCvsCommand();

    *d->singleCvsJob << d->repository->cvsClient() << "-q update";

    if( !recursive )
        *d->singleCvsJob << "-l";

    if( createDirs )
        *d->singleCvsJob << "-d";

    if( pruneDirs )
        *d->singleCvsJob << "-P";

    *d->singleCvsJob << extraOpt << CvsServiceUtils::joinFileList(files)
                     << REDIRECT_STDERR;

    return d->setupNonConcurrentJob();
}


DCOPRef CvsService::watchers(const QStringList& files)
{
    if( !d->hasWorkingCopy() || d->hasRunningJob() )
        return DCOPRef();

    // assemble the command line
    // cvs watchers [FILES]
    d->singleCvsJob->clearCvsCommand();

    *d->singleCvsJob << d->repository->cvsClient() << "watchers"
                     << CvsServiceUtils::joinFileList(files);

    return d->setupNonConcurrentJob();
}


void CvsService::quit()
{
    kapp->quit();
}


CvsJob* CvsService::Private::createCvsJob()
{
    ++lastJobId;

    // create a cvs job
    CvsJob* job = new CvsJob(lastJobId);
    cvsJobs.insert(lastJobId, job);

    job->setRSH(repository->rsh());
    job->setServer(repository->server());
    job->setDirectory(repository->workingCopy());

    return job;
}


DCOPRef CvsService::Private::setupNonConcurrentJob()
{
    singleCvsJob->setRSH(repository->rsh());
    singleCvsJob->setServer(repository->server());
    singleCvsJob->setDirectory(repository->workingCopy());

    return singleJobRef;
}


bool CvsService::Private::hasWorkingCopy()
{
    if( repository->workingCopy().isEmpty() )
    {
        KMessageBox::sorry(0, i18n("You have to set a local working copy "
                                   "directory before you can use this function!"));
        return false;
    }

    return true;
}


bool CvsService::Private::hasRunningJob()
{
    bool result = singleCvsJob->isRunning();

    if( result )
        KMessageBox::sorry(0, i18n("There is already a job running"));

    return result;
}
