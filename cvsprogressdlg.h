/* 
 *  Copyright (C) 1999-2002 Bernd Gehrmann
 *                          bernd@mail.berlios.de
 *
 * This program may be distributed under the terms of the Q Public
 * License as defined by Trolltech AS of Norway and appearing in the
 * file LICENSE.QPL included in the packaging of this file.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */


#ifndef CVSPROGRESSDLG_H
#define CVSPROGRESSDLG_H

#include <qdialog.h>
#include <qstringlist.h>


class KProcess;
class QListBox;
class QPushButton;
class QCString;
class KAnimWidget;


class CvsProgressDialog : public QDialog
{
    Q_OBJECT
    
public:
    CvsProgressDialog( const QString &text, QWidget *parent );
    ~CvsProgressDialog();
    
    bool execCommand(const QString &sandbox, const QString &repository,
                     const QString &cmdline, const QString &errindicator);
    bool getOneLine(QString *str);

protected:
    virtual void closeEvent(QCloseEvent *e);
    
private slots:
    void timeoutOccured();
    void cancelClicked();
    void childExited();
    void receivedOutputNongui(KProcess *proc, char *buffer, int buflen);
    void receivedOutput(KProcess *proc, char *buffer, int buflen);
    
private:
    void stopNonguiPart();
    void startGuiPart();
    void finish();
    bool processOutput();

    bool shown;
    bool cancelled;
    bool hasError;
    QString indic1, indic2;
    KProcess *childproc;
    QListBox *resultbox;
    QPushButton *cancelbutton;
    QTimer *timer;
    QString buf;
    QStringList output;
    KAnimWidget *gear;
};

#endif


// Local Variables:
// c-basic-offset: 4
// End:
