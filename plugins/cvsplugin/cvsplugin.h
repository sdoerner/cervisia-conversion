/*
 * Copyright (c) 2005 Christian Loose <christian.loose@kdemail.net>
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
 * Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef CERVISIA_CVSPLUGIN_H
#define CERVISIA_CVSPLUGIN_H

#include <pluginbase.h>

class CvsService_stub;
class Repository_stub;


namespace Cervisia
{

class CvsCommandBase;
class CvsUpdateParser;


class CvsPlugin : public PluginBase
{
    Q_OBJECT

public:
    CvsPlugin(QObject* parent, const char* name, const QStringList&);
    ~CvsPlugin();

    virtual QString type() const;
    virtual DCOPRef service() const;

    virtual bool canHandle(const KURL& workingCopy);
    virtual void setWorkingCopy(const KURL& workingCopy);
    virtual KURL workingCopy() const;

    virtual QString repository() const;

    virtual void syncWithEntries(const QString& filePath);

    virtual IgnoreFilterBase* filter(const QString& path) const;
    virtual UpdateParser* updateParser() const;

    static CvsService_stub* cvsService() { return m_cvsService; }

public slots:
    void annotate(const QString& fileName,
                  const QString& revision = QString::null);
    void simulateUpdate();

private slots:
    void add();
    void addBinary();
    void addWatch();
    void annotate();
    void commit();
    void createTag();
    void deleteTag();
    void edit();
    void lock();
    void log();
    void remove();
    void removeWatch();
    void revert();
    void unedit();
    void unlock();
    void update();
    void updateToHead();
    void updateToTag();

    void commitRecursive();
    void updateRecursive();

private:
    void executeCommand(CvsCommandBase* cmd);

    void setupMenuActions();
    void startService();

    Repository_stub* m_cvsRepository;
    static CvsService_stub* m_cvsService;
    static CvsUpdateParser* m_updateParser;
};


}


#endif
