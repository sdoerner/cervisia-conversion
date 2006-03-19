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

#ifndef CERVISIA_SVNPLUGIN_H
#define CERVISIA_SVNPLUGIN_H

#include <pluginbase.h>

class SvnService_stub;
class SvnRepository_stub;


namespace Cervisia
{

class CheckoutWidgetBase;
class SvnCommandBase;
class SvnUpdateParser;


class SvnPlugin : public PluginBase
{
    Q_OBJECT

public:
    SvnPlugin(QObject* parent, const char* name, const QStringList&);
    ~SvnPlugin();

    virtual QString type() const;
    virtual DCOPRef service() const;

    virtual bool canHandle(const KURL& workingCopy);
    virtual void setWorkingCopy(const KURL& workingCopy);
    virtual KURL workingCopy() const;

    virtual QString repository() const;

    virtual void syncWithEntries(const QString& path);

    virtual IgnoreFilterBase* filter(const QString& path) const;
    virtual UpdateParser* updateParser() const;

    virtual CheckoutWidgetBase* checkoutWidget(QWidget* parent);

    static SvnService_stub* svnService() { return m_svnService; }

public slots:
    void annotate(const QString& fileName,
                  const QString& revision = QString::null);
    void diff(const QString& fileName,
              const QString& revisionA,
              const QString& revisionB = QString::null);
    void simulateUpdate();
    void checkout(CheckoutWidgetBase* checkoutWidget);
    void view(const QString& fileName,
              const QString& revision);

private slots:
    void add();
    void annotate();
    void commit();
    void diffToBase();
    void diffToHead();
    void log();
    void remove();
    void showLastChange();
    void update();

    void commitRecursive();
    void updateRecursive();

private:
    void executeCommand(SvnCommandBase* cmd);

    void setupMenuActions();
    void startService();

    SvnRepository_stub* m_svnRepository;
    static SvnService_stub* m_svnService;
    static SvnUpdateParser* m_updateParser;
};


}


#endif
