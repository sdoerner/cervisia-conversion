/*
 * Copyright (c) 2005-2006 Christian Loose <christian.loose@kdemail.net>
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

#ifndef CERVISIA_CHECKOUTWIDGET_H
#define CERVISIA_CHECKOUTWIDGET_H

#include <checkoutwidgetbase.h>

class QCheckBox;
class QComboBox;
class KLineEdit;


namespace Cervisia
{

class FetchBranchTagsCommand;


class CheckoutWidget : public CheckoutWidgetBase
{
    Q_OBJECT

public:
    explicit CheckoutWidget(QWidget* parent);
    ~CheckoutWidget();

    QString repository() const;
    QString module() const;
    QString branch() const;
    bool isRecursive() const;
    QString workingFolder() const;
    QString alias() const;
    bool isExportOnly() const;

    bool checkUserInput();

private slots:
    void dirButtonClicked();
    void moduleButtonClicked();
    void branchButtonClicked();
    void branchTextChanged();
    void jobExited(bool normalExit, int status);

private:
    void addRepositories();
    void saveUserInput();
    void restoreUserInput();

    QComboBox*              m_repositoryCombo;
    QComboBox*              m_moduleCombo;
    QComboBox*              m_branchCombo;
    QCheckBox*              m_recursiveChkBox;
    KLineEdit*              m_workFolderEdt;
    KLineEdit*              m_aliasEdt;
    QCheckBox*              m_exportChkBox;
    FetchBranchTagsCommand* m_cmd;
};


}


#endif
