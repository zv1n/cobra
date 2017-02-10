/*******************************************************************************
Copyright (c) 2012, All rights reserved.
	Terry Meacham <terry.t.meacham@gmail.com>
	Andrew Hicks <andrew.t.hicks@gmail.com>
	Nick Huskinson <nhuskinson@gmail.com>
	Jiake Liu  <jiakeliu@gmail.com>
*******************************************************************************/

#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QDialog>

namespace Ui {
    class Preferences;
}

class MainWindow;

class Preferences : public QDialog
{
    Q_OBJECT

public:
    explicit Preferences(QWidget *parent = 0);
    ~Preferences();

    /**
     * @fn void showClientTab()
     * Set the visible tab to be the client tab.
     */
    void showClientTab();

    /**
     * @fn void setWindow(MainWindow* main)
     * Set the window master for these preferences.
     */
    void setWindow(MainWindow* main);

    /**
     * @fn void setVisible(bool)
     * Overload the setVisible() widget function to load values from teh config file.
     */
    void setVisible(bool visible);

    /**
     * @fn void closeEvent(QCloseEvent*)
     * Overload close event handler so that we can notify the mainWindow when the preferences dialog is closed.
     */
    void closeEvent(QCloseEvent *);

protected:
    void setConnectState(bool connected);

private slots:
    void on_lineEditCACert_textChanged(const QString &ca);
    void on_lineEditLocalCert_textChanged(const QString &local);
    void on_lineEditPrivateKey_textChanged(const QString &privkey);
    void on_lineEditUser_textChanged(const QString &username);
    void on_lineEditPass_textChanged(const QString &password);
    void on_tbCACert_clicked();
    void on_tbLocalCert_clicked();
    void on_tbPrivateKey_clicked();
    void on_connectButton_clicked();
    void on_pushButtonStart_clicked();
    void on_guestPwd_textChanged(const QString &arg1);
    void on_participantPwd_textChanged(const QString &arg1);
    void restoreConnection();
    void on_tbStorageDir_clicked();
    void on_localDbEdit_textChanged(const QString &arg1);
    void on_localDbBrowse_clicked();
    void on_timelineDbEdit_textChanged(const QString &arg1);
    void on_timelineDbBrowse_clicked();
    void on_serverDbEdit_textChanged(const QString &arg1);
    void on_serverDbBrowse_clicked();
    void on_localLimit_valueChanged(int arg1);
    void on_serverLimit_valueChanged(int arg1);
    void on_filterMode_currentIndexChanged(const QString &arg1);
    void on_language_currentIndexChanged(const QString &arg1);

private:
    Ui::Preferences *ui;
    MainWindow*     m_mwMain;
    QString         m_sLocalDb;
    QString         m_sTimelineDb;
    QString         m_sServerDb;

};

#endif // PREFERENCES_H
