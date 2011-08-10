/* 
 *  Copyright (c) 2004 Christian Loose <christian.loose@kdemail.net>
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

#include "resolvedlg_p.h"
using namespace Cervisia;

#include <ktextedit.h>


ResolveEditorDialog::ResolveEditorDialog(KConfig& cfg, TQWidget *parent, const char *name)
    : KDialogBase(parent, name, true, TQString(),
                  Ok | Cancel, Ok, true)
    , m_partConfig(cfg)
{
    m_edit = new KTextEdit(this);
    m_edit->setFocus();

    setMainWidget(m_edit);

    TQFontMetrics const fm(fontMetrics());
    setMinimumSize(fm.width('0') * 120,
                   fm.lineSpacing() * 40);

    TQSize size = configDialogSize(m_partConfig, "ResolveEditDialog");
    resize(size);
}


ResolveEditorDialog::~ResolveEditorDialog()
{
    saveDialogSize(m_partConfig, "ResolveEditDialog");
}


void ResolveEditorDialog::setContent(const TQString& text)
{
    m_edit->setText(text);
}


TQString ResolveEditorDialog::content() const
{
    return m_edit->text();
}
