/*
 *  Copyright (C) 1999-2002 Bernd Gehrmann
 *                          bernd@mail.berlios.de
 *  Copyright (c) 2003-2006 Christian Loose <christian.loose@kdemail.net>
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


#ifndef CHECKOUTDLG_H
#define CHECKOUTDLG_H

#include <kdialogbase.h>

class QComboBox;
class QWidgetStack;
class KConfig;

namespace Cervisia { class CheckoutWidgetBase; }


class CheckoutDialog : public KDialogBase
{
public:
    explicit CheckoutDialog(QWidget* parent=0);

    void addCheckoutWidget(const QString& pluginType, Cervisia::CheckoutWidgetBase* w);

    QString pluginType() const;
    Cervisia::CheckoutWidgetBase* currentWidget() const;

protected:
    virtual void slotOk();

private:
    QComboBox*    m_versionControlSystemCombo;
    QWidgetStack* m_widgetStack;
    int           m_id;
};


#endif


// Local Variables:
// c-basic-offset: 4
// End:
