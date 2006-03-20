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


#include "checkoutdlg.h"

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qdir.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qwidgetstack.h>
#include <kprocess.h>
#include <kfiledialog.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kurlcompletion.h>

#include "checkoutwidgetbase.h"
#include "pluginbase.h"
#include "pluginmanager.h"
#include "progressdlg.h"
#include "repositories.h"
#include "misc.h"
#include "cvsservice_stub.h"

#include <kdebug.h>

using Cervisia::CheckoutWidgetBase;
using Cervisia::PluginManager;


CheckoutDialog::CheckoutDialog(QWidget* parent)
    : KDialogBase(parent, 0, true, QString::null,
                  Ok | Cancel | Help, Ok, true)
    , m_id(0)
{
    setCaption( i18n("CVS Checkout") );

    QFrame* mainWidget = makeMainWidget();

    QBoxLayout* layout = new QVBoxLayout(mainWidget, 0, spacingHint());

    m_versionControlSystemCombo = new QComboBox(false, mainWidget);
    layout->addWidget(m_versionControlSystemCombo);

    m_widgetStack = new QWidgetStack(mainWidget);
    m_widgetStack->setFrameStyle( QFrame::GroupBoxPanel | QFrame::Sunken );
    layout->addWidget(m_widgetStack);

    connect( m_versionControlSystemCombo, SIGNAL(activated(int)),
             m_widgetStack, SLOT(raiseWidget(int)) );
}


void CheckoutDialog::addCheckoutWidget(const QString& pluginType, Cervisia::CheckoutWidgetBase* w)
{
    m_versionControlSystemCombo->insertItem(pluginType);
    m_widgetStack->addWidget(w, m_id);
    ++m_id;
}


QString CheckoutDialog::pluginType() const
{
    return m_versionControlSystemCombo->currentText();
}


Cervisia::CheckoutWidgetBase* CheckoutDialog::currentWidget() const
{
    return static_cast<CheckoutWidgetBase*>(m_widgetStack->visibleWidget());
}


void CheckoutDialog::slotOk()
{
    if( !currentWidget()->checkUserInput() )
        return;

    KDialogBase::slotOk();
}


// Local Variables:
// c-basic-offset: 4
// End:
