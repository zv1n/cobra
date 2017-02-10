/*******************************************************************************
Copyright (c) 2012, All rights reserved.
	Terry Meacham <terry.t.meacham@gmail.com>
	Andrew Hicks <andrew.t.hicks@gmail.com>
	Nick Huskinson <nhuskinson@gmail.com>
	Jiake Liu  <jiakeliu@gmail.com>
*******************************************************************************/

#include "preferences.h"
#include "ui_preferences.h"
#include "mainwindow.h"
#include "net.h"
#include "debug.h"
#include <QtGui>


QSettings* g_cobra_settings;

Preferences::Preferences(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Preferences),
    m_mwMain(NULL)
{
    ui->setupUi(this);

    QRegExp unameRexp("[a-zA-Z0-9_]+");
    ui->lineEditUser->setValidator(new QRegExpValidator(unameRexp, this));
    ui->lineEditPort->setValidator(new QIntValidator(1024, 65535, this));
    ui->lineEditPort_2->setValidator(new QIntValidator(1024, 65535, this));
    //QRegExp ipv4_RegEx(IPV4_REGEX);
    //QRegExp ipv4_RegEx("(((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)([.]|$)){1,4})|(([^0-2][^0-9][^0-9])(([a-zA-Z0-9_])+[.])+)");
    QRegExp ipv4_RegEx(IPV4_REGEX"|(([^0-2][^0-9][^0-9])(([a-zA-Z0-9_])+[.])+)");
    //QRegExp ipv4_RegEx(HOST_NAME);
    ui->lineEditIP->setValidator(new QRegExpValidator(ipv4_RegEx, this));

    QObject::connect(cobraNetHandler::instance(), SIGNAL(rejected()), this, SLOT(restoreConnection()));
}

Preferences::~Preferences()
{
    delete ui;
}

void
Preferences::showClientTab()
{
    ui->prefTab->setCurrentIndex(1);
    show();
}

void
Preferences::setVisible(bool visible)
{
    ui->lineEditCACert->setText(g_cobra_settings->value("ssl/ca").toString());
    ui->lineEditLocalCert->setText(g_cobra_settings->value("ssl/local_certificate").toString());
    ui->lineEditPrivateKey->setText(g_cobra_settings->value("ssl/private_key").toString());
    ui->lineEditUser->setText(g_cobra_settings->value("client/username").toString());
    ui->lineEditPass->setText(g_cobra_settings->value("client/password").toString());
    ui->guestPwd->setText(g_cobra_settings->value("server/guest").toString());
    ui->participantPwd->setText(g_cobra_settings->value("server/participant").toString());
    ui->storageDir->setText(g_cobra_settings->value("storage_dir").toString());

    ui->localLimit->setValue(g_cobra_settings->value("limits/local").toUInt()>>20);
    ui->serverLimit->setValue(g_cobra_settings->value("limits/server").toUInt()>>20);

    m_sLocalDb = g_cobra_settings->value("db/local").toString();
    ui->localDbEdit->setText(m_sLocalDb);
    m_sTimelineDb = g_cobra_settings->value("db/timeline").toString();
    ui->timelineDbEdit->setText(m_sTimelineDb);
    m_sServerDb = g_cobra_settings->value("db/server").toString();
    ui->serverDbEdit->setText(m_sServerDb);

    QString mode = g_cobra_settings->value("filter").toString();
    int index = ui->filterMode->findText(mode);

    if (mode.isEmpty() || !mode.length()) {
      mode = ui->filterMode->itemText(0);
      g_cobra_settings->setValue("filter", mode);
      index = 0;
    }

    ui->localCurrentSize->setText(QString("%1 KB").arg(m_mwMain->localSize()>>10));
    ui->serverCurrentSize->setText(QString("%1 KB").arg(m_mwMain->serverSize()>>10));
    ui->filterMode->setCurrentIndex(index);

    QString lang = g_cobra_settings->value("language").toString();
    int lindex = ui->language->findText(lang);

    if(lang.isEmpty() || !lang.length()) {
        lang = ui->language->itemText(0);
        g_cobra_settings->setValue("language", lang);
        lindex = 0;
    }

    ui->language->setCurrentIndex(lindex);

    QDialog::setVisible(visible);
}

void
Preferences::setConnectState(bool connected)
{
    const char* temp = (connected)?"true":"false";

    debug(LOW, "Setting the connected state to %s\n", temp);
    debug(HIGH, "Connecting as %s\n", qPrintable(ui->lineEditUser->text()));
    ui->Client->setEnabled(!connected);
    ui->Server->setEnabled(!connected);
}

void
Preferences::on_lineEditCACert_textChanged(const QString &ca)
{
    g_cobra_settings->setValue("ssl/ca", ca);
}

void
Preferences::on_lineEditLocalCert_textChanged(const QString &local)
{
    g_cobra_settings->setValue("ssl/local_certificate", local);
}

void
Preferences::on_lineEditPrivateKey_textChanged(const QString &privkey)
{
    g_cobra_settings->setValue("ssl/private_key", privkey);
}

void
Preferences::on_lineEditUser_textChanged(const QString &username)
{
    g_cobra_settings->setValue("client/username", username);
}

void
Preferences::on_lineEditPass_textChanged(const QString &password)
{
    g_cobra_settings->setValue("client/password", password);
}


void
Preferences::on_tbCACert_clicked()
{
    QString path;

    path = QFileDialog::getOpenFileName(this,
                                        "Open CA Certificate",
                                        QString::null,
                                        QString::null);
    ui->lineEditCACert->setText(path);
}

void
Preferences::on_tbLocalCert_clicked()
{
    QString path;

    path = QFileDialog::getOpenFileName(this,
                                        "Open Local Certificate",
                                        QString::null,
                                        QString::null);
    ui->lineEditLocalCert->setText(path);
}

void
Preferences::on_tbPrivateKey_clicked()
{
    QString path;

    path = QFileDialog::getOpenFileName(this,
                                        "Open Private Key",
                                        QString::null,
                                        QString::null);
    ui->lineEditPrivateKey->setText(path);
}


void
Preferences::on_connectButton_clicked()
{
    cobraNetHandler* cnd = cobraNetHandler::instance();

    if (!cnd->loadClientCertificates()) {
        QMessageBox::warning(this, "Certificates Not Specified!",
                             "Before you can connect to a server, you must select\n"
                             "the CA Certificate to use with the connection!\n"
                             "(Select \"Preferences->Profile\")");
        return;
    }

    if (ui->checkBoxIgnoreErr->isChecked()) {
        QList<QSslError> exclude;
        cobraNetHandler::instance()->getAllowedErrors(exclude);

        exclude.append(QSslError(QSslError::HostNameMismatch, cobraNetHandler::instance()->getCaCertificate()));

        cobraNetHandler::instance()->setAllowedErrors(exclude);
    }

    debug(HIGH, "Connecting as %s\n", qPrintable(ui->lineEditUser->text()));

    cnd->setUsername(ui->lineEditUser->text());
    cnd->setPassword(ui->lineEditPass->text());

    bool result = cnd->connect(ui->lineEditIP->text(), ui->lineEditPort->text().toInt());
    debug(CRITICAL, "Connect: %s\n", result ? "Connection Successful!" : "Failed to Connect!");

    if (!result)
        QMessageBox::critical(this, "Failed to Connect to Server!",
                              "Failed to properly connect to the server! (Is the port or IP incorrect?)");

    setConnectState(result);
}



void
Preferences::on_pushButtonStart_clicked()
{
    cobraNetHandler* cnd = cobraNetHandler::instance();

    if (!cnd->loadServerCertificates(ui->lineEditKey->text())) {
        QMessageBox::warning(this, "Certificates Not Specified!",
                             "Before you can connect to a server, you must select\n"
                             "the CA Certificate, Local Certificate, and Private Key\n"
                             "to use with the connection!\n(Select \"Preferences->Profile\")");
        return;
    }

    if (ui->lineEditPort_2->text().toInt() < 1024 && getuid() != 0) {
        QMessageBox::warning(this, "Invalid Server Port Specified!",
                             "Invalid Port Specified! (Port must be >1024 without Superuser!)");
        return;
    }

    cnd->setUsername(ui->lineEditUser->text());

    debug(HIGH, "Setting Participant Passphrase: %s\n", qPrintable(ui->participantPwd->text()));
    debug(HIGH, "Setting Guest Passphrase: %s\n", qPrintable(ui->guestPwd->text()));

    bool result = cnd->listen(QHostAddress::Any, ui->lineEditPort_2->text().toInt());
    debug(CRITICAL, "Listen: %s\n", result ? "Listen Successful!" : "Failed to Listen!");

    if (!result)
        QMessageBox::critical(this, "Failed to Start Server!",
                              "Failed to properly start the server! (Is your port in use?)");

    setConnectState(result);
}

void Preferences::on_guestPwd_textChanged(const QString &pwd)
{
    g_cobra_settings->setValue("server/guest", pwd);
    cobraNetHandler::instance()->setGuestPassword(pwd);
}

void Preferences::on_participantPwd_textChanged(const QString &pwd)
{
    g_cobra_settings->setValue("server/participant", pwd);
    cobraNetHandler::instance()->setSessionPassword(pwd);
}

void Preferences::restoreConnection()
{
    setConnectState(false);
}

void Preferences::on_tbStorageDir_clicked()
{
    QString path;

    path = QFileDialog::getExistingDirectory(this,
                                             "Select Storage Directory",
                                             QString::null);

    g_cobra_settings->setValue("storage_dir", path);
    ui->storageDir->setText(path);
}

void Preferences::on_localDbEdit_textChanged(const QString &localdb)
{
    g_cobra_settings->setValue("db/local", localdb);
}

void Preferences::on_localDbBrowse_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Select Local File"),
                                                    "",
                                                    tr("SQL Database (*.db)"));
    cobraClipList list;
    if (!list.setName(fileName)) {
	QMessageBox::warning(this, tr("Invalid Database!"), 
		tr("Specified file list was invalid!"));
    } else {
    	g_cobra_settings->setValue("db/local", fileName);
    	ui->localDbEdit->setText(fileName);
    }
}

void Preferences::on_timelineDbEdit_textChanged(const QString &localdb)
{
    g_cobra_settings->setValue("db/timeline", localdb);
}

void Preferences::on_timelineDbBrowse_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Select Timeline File"),
                                                    "",
                                                    tr("SQL Database (*.db)"));

    cobraTimelineList list;
    if (!list.setName(fileName)) {
	QMessageBox::warning(this, tr("Invalid Database!"), 
		tr("Specified timeline file was invalid!"));
    } else {
    	g_cobra_settings->setValue("db/timeline", fileName);
    	ui->timelineDbEdit->setText(fileName);
    }
}

void Preferences::on_serverDbEdit_textChanged(const QString &localdb)
{
    g_cobra_settings->setValue("db/server", localdb);
}

void Preferences::on_serverDbBrowse_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Select Server File"),
                                                    "",
                                                    tr("SQL Database (*.db)"));

    cobraClipList list;
    if (!list.setName(fileName)) {
	QMessageBox::warning(this, tr("Invalid Database!"), 
		tr("Specified list file was invalid!"));
    } else {
	g_cobra_settings->setValue("db/server", fileName);
	ui->serverDbEdit->setText(fileName);
    }
}

void
Preferences::closeEvent(QCloseEvent *event)
{
    (void) event;

    if (ui->localDbEdit->text() != m_sLocalDb)
        m_mwMain->updateLocalListDb(ui->localDbEdit->text());
    if (ui->timelineDbEdit->text() != m_sTimelineDb)
        m_mwMain->updateTimelineDb(ui->timelineDbEdit->text());
    if (ui->serverDbEdit->text() != m_sServerDb)
        m_mwMain->updateServerDb(ui->serverDbEdit->text());
}

void
Preferences::setWindow(MainWindow *main)
{
    m_mwMain = main;
}

void Preferences::on_localLimit_valueChanged(int arg1)
{
  if (arg1 < 0) {
    arg1 = 0;
    ui->localLimit->setValue(0);
  }

  int val = g_cobra_settings->value("limits/local").toInt();

  g_cobra_settings->setValue("limits/local", arg1 << 20);
  if (m_mwMain)
      if (m_mwMain->localLimitReached(0)) {
          QMessageBox::warning(this, "Invalid Limit Specified", "Please specificy a local limit above current usage!");
          ui->localLimit->setValue(val>>20);
          g_cobra_settings->setValue("limits/local", val);
      }
}

void Preferences::on_serverLimit_valueChanged(int arg1)
{
  if (arg1 < 0) {
    arg1 = 0;
    ui->serverLimit->setValue(0);
  }
  int val = g_cobra_settings->value("limits/server").toInt();

  g_cobra_settings->setValue("limits/server", arg1 << 20);
  if (m_mwMain)
      if (m_mwMain->serverLimitReached(0)) {
          QMessageBox::warning(this, "Invalid Limit Specified", "Please specificy a server limit above current usage!");
          ui->localLimit->setValue(val>>20);
          g_cobra_settings->setValue("limits/server", val);
      }
}

void Preferences::on_filterMode_currentIndexChanged(const QString &arg1)
{
  g_cobra_settings->setValue("filter", arg1);
}

void Preferences::on_language_currentIndexChanged(const QString &arg1)
{
  g_cobra_settings->setValue("language", arg1);
}
