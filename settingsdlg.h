/* 
 *  Copyright (C) 1999-2001 Bernd Gehrmann
 *                          bernd@physik.hu-berlin.de
 *
 * This program may be distributed under the terms of the Q Public
 * License as defined by Trolltech AS of Norway and appearing in the
 * file LICENSE.QPL included in the packaging of this file.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */


#ifndef _SETTINGSDLG_H_
#define _SETTINGSDLG_H_

#include <kdialogbase.h>

class QCheckBox;
class QComboBox;
class QHButtonGroup;
class QRadioButton;
class KIntegerLine;
class KIntNumInput;
class KLineEdit;
class KConfig;

class FontButton : public QPushButton
{
    Q_OBJECT
public:
    FontButton( const QString &text, QWidget *parent=0, const char *name=0 );

private slots:
    void chooseFont();
};


class SettingsDialog : public KDialogBase
{
    Q_OBJECT

public:
    SettingsDialog( KConfig *conf, QWidget *parent=0, const char *name=0 );

protected slots:
     virtual void done(int res);

private:
    void readSettings();
    void writeSettings();

    KConfig *config;
    KIntNumInput *timeoutedit;
    KIntNumInput *contextedit;
    KIntNumInput *tabwidthedit;
    KLineEdit *cvspathedit;
    QComboBox *compressioncombo;
    KLineEdit *usernameedit;
    KLineEdit *editoredit;
    KLineEdit *diffoptedit;
    KLineEdit *extdiffedit;
    QCheckBox *remotestatusbox;
    QCheckBox *localstatusbox;
#if 0
    QCheckBox *usedcopbox;
    KLineEdit *clientedit;
    KLineEdit *objectedit;
#endif
    FontButton *protocolfontbox;
    FontButton *annotatefontbox;
    FontButton *difffontbox;
    QCheckBox *splitterbox;
};

#endif


// Local Variables:
// c-basic-offset: 4
// End:
