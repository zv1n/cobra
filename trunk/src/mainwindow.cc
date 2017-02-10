/*******************************************************************************
Copyright (c) 2012, All rights reserved.
	Terry Meacham <terry.t.meacham@gmail.com>
	Andrew Hicks <andrew.t.hicks@gmail.com>
	Nick Huskinson <nhuskinson@gmail.com>
	Jiake Liu  <jiakeliu@gmail.com>
*******************************************************************************/

#include "mainwindow.h"
#include "ui_mainwindow_docks.h"

#include "debug.h"
#include "net.h"
#include "filevalidator.h"
#include "preferences.h"
#include "transfersdlg.h"
#include "treewidget.h"
#include "clipdialog.h"
#include "timelinedialog.h"
#include "markerdialog.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_pDialog(NULL), m_dTransfers(NULL),
    m_cclFocused(NULL), m_ccdDialog(NULL),
    m_tlDialog(NULL), m_mDialog(NULL)
#if !defined(NO_AUDIO)
   ,m_aoAudioOutput(NULL), m_moAudioFile(NULL)
#endif
{
    ui->setupUi(this);

    cobraNetHandler::instance()->init();

    installEventFilter(cobraNetHandler::instance());

    tabifyDockWidget(ui->serverList, ui->fileList);
    //tabifyDockWidget(ui->fileList, ui->cueList);
    ui->cueList->setVisible(false);
    ui->markDock->setVisible(false);

    ui->timelineTree->configure();

    ui->serverTree->showUpload(false);
    ui->serverTree->showDownload(true);
    ui->serverTree->showUid(false);
    ui->serverTree->configure();

    QString localdb = g_cobra_settings->value("db/local").toString();
    if (localdb.isEmpty()) {
        localdb = QDir::currentPath() + "/local.db";
        g_cobra_settings->setValue("db/local", localdb);
    }

    QString serverdb = g_cobra_settings->value("db/server").toString();
    if (serverdb.isEmpty()) {
        serverdb = QDir::currentPath() + "/server.db";
        g_cobra_settings->setValue("db/server", serverdb);
    }

    QString timelinedb = g_cobra_settings->value("db/timeline").toString();
    if (timelinedb.isEmpty()) {
        timelinedb = QDir::currentPath() + "/timeline.db";
        g_cobra_settings->setValue("db/timeline", timelinedb);
    }

    QString storage = g_cobra_settings->value("storage_dir").toString();
    if (storage.isEmpty()) {
        storage = QDir::currentPath() + "/";
        g_cobra_settings->setValue("storage_dir", storage);
    }

    ui->localTree->showUpload(true);
    ui->localTree->showDownload(false);
    ui->localTree->showUid(false);
    ui->localTree->setName(localdb);
    ui->localTree->configure();

    ui->timelineTree->hideColumn(3);
    ui->timelineTree->hideColumn(4);

    QString style =
            "QDockWidget::title { "
            "border-top-left-radius: 0px; "
            "border-top-right-radius: 0px; "
            "border-bottom-left-radius: 0px; "
            "border-bottom-right-radius: 0px; "
            "border: 1px solid #202020; "
            "color: rgb(255,255,255); "
            "background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:0.4,"
            "stop:0.856444 rgb(115,115,115), stop:0.3 rgb(100,100,100), stop:0.980424 rgb(92,92,92)); "
            "text-align: center; "
            "padding-left: 5px; "
            "}";

    setStyleSheet(style);
    QList<QDockWidget*> docks = this->findChildren<QDockWidget*>();

    for (int x=0; x<docks.count(); x++) {
        docks.at(x)->setStyleSheet(style);
    }

    ui->sendText->installEventFilter(this);
    ui->serverTree->installEventFilter(this);
    ui->localTree->installEventFilter(this);

    cobraStateEventHandler* stateHandler = new cobraStateEventHandler();
    cobraChatEventHandler* chatHandler = new cobraChatEventHandler();
    cobraAuthEventHandler* authHandler = new cobraAuthEventHandler();
    cobraTransferEventHandler* xferHandler = new cobraTransferEventHandler();
    cobraClipUpdateEventHandler* clipupdate = new cobraClipUpdateEventHandler();
    cobraTimelineEventHandler* tline = new cobraTimelineEventHandler();
    cobraPlayEventHandler* playhandler = new cobraPlayEventHandler();

    m_ctcTransfer = xferHandler;
    xferHandler->setWindow(this);
    playhandler->setWindow(this);

    chatHandler->setChatDock(ui->chatDock);
    clipupdate->setLocalList(ui->localTree);
    clipupdate->setServerList(ui->serverTree);
    tline->setTimelineList(ui->timelineTree);

    cobraNetHandler::instance()->registerEventHandler(stateHandler);
    cobraNetHandler::instance()->registerEventHandler(chatHandler);
    cobraNetHandler::instance()->registerEventHandler(authHandler);
    cobraNetHandler::instance()->registerEventHandler(xferHandler);
    cobraNetHandler::instance()->registerEventHandler(clipupdate);
    cobraNetHandler::instance()->registerEventHandler(tline);
    cobraNetHandler::instance()->registerEventHandler(playhandler);

    registerMetaCommand(&MainWindow::setName, "\\name");
    registerMetaCommand(&MainWindow::metaConnect, "\\connect");
    registerMetaCommand(&MainWindow::setAway, "\\away");

    QObject::connect(ui->sendButton, SIGNAL(clicked()), this, SLOT(sendChat()));
    QObject::connect(cobraNetHandler::instance(), SIGNAL(connected(bool)), ui->actionSync, SLOT(setEnabled(bool)));
    QObject::connect(cobraNetHandler::instance(), SIGNAL(connected(bool)), ui->actionAddMark, SLOT(setEnabled(bool)));
    QObject::connect(cobraNetHandler::instance(), SIGNAL(connected(bool)), ui->actionAddTimeline, SLOT(setEnabled(bool)));
    QObject::connect(cobraNetHandler::instance(), SIGNAL(connected(bool)), this, SLOT(connected(bool)));
    QObject::connect(cobraNetHandler::instance(), SIGNAL(newConnection(int)),this, SLOT(newConnection(int)));

    QObject::connect(ui->serverTree, SIGNAL(downloadSelected(bool)), ui->actionDownload, SLOT(setEnabled(bool)));

    QObject::connect(ui->serverClear, SIGNAL(clicked()), ui->serverFilter, SLOT(clear()));
    QObject::connect(ui->serverFilter, SIGNAL(textChanged(QString)), ui->serverTree, SLOT(setFilter(QString)));
    QObject::connect(ui->fileClear, SIGNAL(clicked()), ui->fileFilter, SLOT(clear()));
    QObject::connect(ui->fileFilter, SIGNAL(textChanged(QString)), ui->localTree, SLOT(setFilter(QString)));

    stateHandler->put();
    chatHandler->put();
    authHandler->put();
    xferHandler->put();
    clipupdate->put();
    playhandler->put();
    tline->put();

    /* Hide the central widget so that the dock widgets take over. */
    ui->centralwidget->hide();

    QRect rect = geometry();
    rect.setHeight(500);
    setGeometry(rect);
}

bool
MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    QKeyEvent* key = NULL;

    if (!obj || !event)
        return QMainWindow::eventFilter(obj, event);

    if (event->type() != QEvent::KeyPress)
        goto out;

    key = static_cast<QKeyEvent*>(event);
    if (key->key() != Qt::Key_Enter && key->key() != Qt::Key_Return)
        goto out;

    key->ignore();
    if (sendChat())
        return true;

out:
    focusFilter(obj, event);
    return QMainWindow::eventFilter(obj, event);
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool
MainWindow::localLimitReached(unsigned long add) const
{
    unsigned long size = g_cobra_settings->value("limits/local").toUInt();
    debug(ERROR(CRITICAL), "Local Limits: %u\n", size);
    if (size <= 0)
        return false;

    unsigned long tsize = ui->localTree->totalSize() + add;
    debug(ERROR(CRITICAL), "Local Limits: %u\n", tsize);
    if (size > tsize)
        return false;

    return true;
}

unsigned long
MainWindow::localSize() const
{
    return ui->localTree->totalSize();
}

unsigned long
MainWindow::serverSize() const
{
    return ui->serverTree->totalSize();
}

bool
MainWindow::serverLimitReached(unsigned long add) const
{
    unsigned long size = g_cobra_settings->value("limits/server").toUInt();
    debug(ERROR(CRITICAL), "Server Limits: %u\n", size);
    if (size <= 0)
        return false;

    unsigned long tsize = ui->serverTree->totalSize() + add;
    debug(ERROR(CRITICAL), "Server Limits: %u\n", tsize);
    if (size > tsize)
        return false;

    return true;
}

void
MainWindow::connected(bool cn)
{
    if (!cn) {
        debug(ERROR(CRITICAL), "Disconnected!\n");
        return;
    }

    if (!cobraNetHandler::instance()->isServing())
        return;

    QString serverDb = g_cobra_settings->value("db/server").toString();
    QString timelineDb = g_cobra_settings->value("db/timeline").toString();
    ui->serverTree->setName(serverDb);
    ui->timelineTree->setName(timelineDb);

    cobraTimeline cur = ui->timelineTree->getCurrentTimeline();
    if (cur.getUid() <= 0) {
        return;
    }

    QDateTime tl = cur.getStartTime();
    cobraNetEventHandler* handler = cobraNetHandler::instance()->getEventHandler(cobraTimelineEventType);
    if (handler) {
        cobraTimelineEventHandler* timelinehandler = static_cast<cobraTimelineEventHandler*>(handler);
        timelinehandler->setLastTimeline(tl);
        handler->put();
    }

}

void
MainWindow::newConnection(int idx)
{
    if (!cobraNetHandler::instance()->isServing())
        return;

    /* SErver MOtD whatever else, can be handled here... */

    cobraTimeline cur = ui->timelineTree->getCurrentTimeline();
    if (cur.getUid() <= 0) {
        return;
    }

    cobraTimelineEvent* event = new cobraTimelineEvent;

    event->setTimeline(cur);
    event->setCommand(cobraTimelineEvent::GenerateTimeline);
    event->setSource(SERVER);
    event->setResponse(true);
    event->setDestination(idx);

    cobraNetHandler::instance()->sendEvent(event);
}

bool
MainWindow::setName(QString name)
{
    QRegExp exp("\\s+");

    QStringList cmdArgs = name.split(exp);
    if (cmdArgs.count() < 1)
        return false;

    QRegExp unameRexp("[a-zA-Z0-9_]+");
    if (!unameRexp.exactMatch(cmdArgs[0])) {
        ui->chatText->append(CHAT_NOTIFY("Failed to set name ('A-Z', 'a-z', '0-9', and '_' only!)"));
        return false;
    }

    m_cUsername = cmdArgs[0];
    return true;
}

bool
MainWindow::metaConnect(QString cmd)
{
    QRegExp exp("\\s+");

    QStringList cmdArgs = cmd.split(exp);
    if (cmdArgs.count() < 4)
        return false;

    cobraNetHandler* handler = cobraNetHandler::instance();

    handler->setUsername(cmdArgs[2]);
    handler->setPassword(cmdArgs[3]);

    handler->connect(cmdArgs[0], cmdArgs[1].toInt());
    return true;
}

bool
MainWindow::setAway(QString msg)
{
    debug(LOW, "Settings Away MSG: %s\n", qPrintable(msg));
    ui->chatText->append(CHAT_NOTIFY("Settings away:"));
    ui->chatText->append(msg);
    return true;
}


bool
MainWindow::sendChat()
{
    cobraChatEvent* chat = NULL;
    QString text;

    if (!cobraNetHandler::instance()->isConnected()) {
        debug(ERROR(HIGH), "Attempted to send chat while not connected\n");
        ui->sendText->setText("");
        ui->chatText->append(CHAT_NOTIFY("Failed to send message: Not Connected."));
        return false;
    }

    text = ui->sendText->toPlainText();
    text.remove('\n');

    chat = new cobraChatEvent();
    if (!chat)
        return false;

    if (text.isEmpty()) {
        ui->sendText->setText("");
        return false;
    }

    if (text[0] == '\\') {
        ui->sendText->setText("");
        return processMetaCommand(text);
    }

    chat->setDestination(SERVER);
    chat->setSource(cobraMyId);

    chat->setMsg(text);
    cobraSendEvent(chat);

    ui->sendText->setText("");
    return true;
}

bool
MainWindow::registerMetaCommand(metaCmdEntry func, QString cmd)
{
    if (m_msfMetaMap[cmd] != NULL) {
        debug(ERROR(LOW), "Command '%s' already registered.\n", qPrintable(cmd));
        return false;
    }

    m_msfMetaMap[cmd] = func;
    debug(LOW, "Command set.\n");
    return true;
}

bool
MainWindow::processMetaCommand(QString cmd)
{
    QString args;
    QRegExp exp;
    int pos = 0;

    exp.setPattern("\\s");
    pos = cmd.indexOf(exp);

    if (pos != -1) {
        args = cmd.right(cmd.length() - pos);
        cmd = cmd.left(pos);

        exp.setPattern("\\S");
        pos = args.indexOf(exp);
        if (pos != -1)
            args = args.right(args.length() - (pos));

        debug(LOW, "Command (%d): '%s' Args: '%s'\n", pos, qPrintable(cmd), qPrintable(args));
    }

    if (m_msfMetaMap[cmd] == NULL) {
        debug(ERROR(LOW), "Failed to find command: %s!\n", qPrintable(cmd));
        return false;
    }

    debug(LOW, "Found command!\n");
    metaCmdEntry fn = m_msfMetaMap[cmd];
    return (this->*fn)(args);
}

void
MainWindow::on_actionPreferences_triggered()
{
    debug(HIGH, "Show preferences...\n");
    if (!m_pDialog) {
        m_pDialog = new Preferences(this);
        m_pDialog->setWindow(this);
    }
    m_pDialog->show();
}

void
MainWindow::on_actionConnect_triggered()
{
    debug(HIGH, "Show connection preferences...\n");
    if (!m_pDialog) {
        m_pDialog = new Preferences(this);
        m_pDialog->setWindow(this);
    }

    m_pDialog->showClientTab();
}

void
MainWindow::on_actionFile_List_toggled(bool checked)
{
    ui->fileList->setVisible(checked);
    checked = !checked;
}


void
MainWindow::on_actionClip_Cue_toggled(bool checked)
{
    ui->cueList->setVisible(checked);
    checked = !checked;
}

void
MainWindow::on_actionServer_list_toggled(bool checked)
{
    ui->serverList->setVisible(checked);
    checked = !checked;
}

void
MainWindow::on_actionChat_Window_toggled(bool checked)
{
    ui->chatDock->setVisible(checked);
    checked = !checked;
}

void
MainWindow::on_actionFile_Info_toggled(bool checked)
{
    ui->markDock->setVisible(checked);
    checked=!checked;
}

void MainWindow::on_actionTransfers_triggered()
{
    if (!m_dTransfers)
        m_dTransfers = new Transfersdlg;
    m_dTransfers->show();
}

void MainWindow::on_actionSelectUpload_triggered()
{
    if (cobraNetHandler::instance()->isServing())
        return;

    QString path = QFileDialog::getOpenFileName(this, "Send File");
    if (!QFile::exists(path)) {
        return;
    }

    cobraTransferFile* file = new cobraTransferFile(path);
    file->setDestination(SERVER);
    file->setSource(cobraMyId);
    file->setSending(true);

    cobraTransferEvent* event = new cobraTransferEvent;
    event->fromFile(file);
    event->setResponse(false);
    event->setExtension(path);

    cobraTransferController::addPendingTransfer(file);
    cobraSendEvent(event);
}

bool
MainWindow::focusFilter(QObject* obj, QEvent* event)
{
    QString objName = obj->objectName();
    if (objName != ui->localTree->objectName() &&
            objName != ui->serverTree->objectName())
        return false;

    QTreeWidget* wdt = qobject_cast<QTreeWidget*>(obj);
    if (!wdt)
        return false;

    if (event->type() == QEvent::FocusIn) {
        m_cclFocused = dynamic_cast<cobraTreeWidget*>(obj);
        ui->actionRemoveClip->setEnabled(true);
        ui->actionEditClip->setEnabled(true);
        ui->actionPlay->setEnabled(true);
    } else
        if (event->type() == QEvent::FocusOut) {
            m_cclFocused = NULL;
            ui->actionRemoveClip->setEnabled(false);
            ui->actionEditClip->setEnabled(false);
            ui->actionPlay->setEnabled(false);

        } else {

            if (!m_cclFocused)
                return false;

            cobraTreeWidget* widget = dynamic_cast<cobraTreeWidget*>(m_cclFocused);

            if (!widget)
                return false;

            QList<QTreeWidgetItem*> selection = widget->selectedItems();

            if(selection.size() > 0) {
                ui->actionEditClip->setEnabled(true);
                ui->actionPlay->setEnabled(true);
                ui->actionRemoveClip->setEnabled(m_cclFocused == ui->localTree);
            } else {
                ui->actionEditClip->setEnabled(false);
                ui->actionPlay->setEnabled(false);
                ui->actionRemoveClip->setEnabled(false);
            }
        }

    return false;
}

void
MainWindow::on_actionAddClip_triggered()
{
    int res = 0;
    bool isNew = true;
    cobraClipList *list = ui->localTree;
    QString path = QFileDialog::getOpenFileName(this, tr("Open File"),QDir::homePath(), QString::null);

    if (path.isEmpty() || path.isNull())
        return;

    QFileInfo file(path);
    QByteArray cliphash = cobraTransferFile::hashFile(path);

    if (localLimitReached(file.size())) {
        /* If the limit is reached, notify the user and
          delete file; */
        QMessageBox::warning(this, tr("Size Limits"), tr("Local size limit reached!\n(See 'Preferences->Limits->Local Limits'\n"
                                                         "to change the on disk size limit."));
        return;
    }

    cobraClip clip;
    clip.setTitle("<title>");
    clip.setPath(path);
    clip.setHash(cliphash);
    clip.setSize(file.size());
    clip.setExtension(file.completeSuffix());

    if (list->containsHash(clip.getHash())) {
        int btn = QMessageBox::question(this, tr("Unable to Add Clip"), tr("Clip already exists! Would you like to edit the existing entry?"),
                                        QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes);
        if (btn == QMessageBox::Yes) {
            clip = list->getClipByHash(clip.getHash());
            isNew = false;
        } else return;
    } else if (!list->addClip(clip)) {
        QMessageBox::critical(this, tr("Unable to Add Clip"), tr("Failed to add the clip!"));
        return;
    }

    if (clip.getUid() == 0) {
        QMessageBox::critical(this, tr("Unable to Add Clip"), tr("Failed to add the clip!"));
        return;
    }

    if (!m_ccdDialog)
        m_ccdDialog = new cobraClipDialog;

    m_ccdDialog->setClipList(list);
    if (!m_ccdDialog->setClip(clip.getUid())) {
        QMessageBox::critical(this, tr("Unable to Add Clip"), tr("Unable to edit specified clip!"));
        return;
    }

    m_ccdDialog->setModal(true);

    m_ccdDialog->exec();
    res = m_ccdDialog->result();

    m_ccdDialog->setClipList(NULL);

    if (res == QDialog::Rejected)
        list->removeClip(clip.getUid());
    else if (isNew){
        ui->localTree->setUpload(clip.getUid(), true);
    }
}

void
MainWindow::on_actionRemoveClip_triggered()
{
    QMessageBox msgBox;
    msgBox.setText(tr("Are you sure you want to remove the selected clip?"));
    msgBox.setWindowTitle(tr("Warning"));
    msgBox.setModal(true);

    QPushButton *noButton = msgBox.addButton(trUtf8("     No     "), QMessageBox::YesRole);
    QPushButton *yesButton = msgBox.addButton(trUtf8("    Yes   "), QMessageBox::YesRole);

    msgBox.setDefaultButton(noButton);
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.exec();

    if (msgBox.clickedButton() == noButton) {

        debug(ERROR(LOW), "Canceled clip removal\n");
        msgBox.accept();

    } else if (msgBox.clickedButton() == yesButton) {

        debug(ERROR(LOW), "Clip removal confirmed\n");
        QTreeWidgetItem* ci = ui->localTree->currentItem();

        if (ci!=NULL) {
            int uid = ci->text(cobraTreeWidget::ID).toInt(0,10);
            ui->localTree->removeClip(uid);
        } else return;
    }
}

void
MainWindow::on_actionEditClip_triggered()
{
    if (!m_cclFocused) {
        debug(ERROR(CRITICAL), "Failed to find associated List!");
        ui->actionEditClip->setEnabled(false);
        return;
    }

    cobraTreeWidget *clw =  dynamic_cast<cobraTreeWidget*>(m_cclFocused);

    if (!clw)
        return;

    QTreeWidgetItem* ci = clw->currentItem();

    if (!ci) {
        debug(ERROR(CRITICAL), "Failed to find associated list item!");
        ui->actionEditClip->setEnabled(false);
        return;
    }

    int id = ci->text(cobraTreeWidget::ID).toInt();
    debug(HIGH, "Edit Clip: %d\n", id);

    if (!m_ccdDialog)
        m_ccdDialog = new cobraClipDialog;

    m_ccdDialog->setClipList(clw);

    if (!m_ccdDialog->setClip(id)) {
        QMessageBox::critical(this, tr("Unable to Edit Clip"), tr("Unable to edit specified clip!"));
        return;
    }

    m_ccdDialog->setModal(true);

    m_ccdDialog->exec();
    m_ccdDialog->result();

    m_ccdDialog->setClipList(NULL);
}

void
MainWindow::on_actionDownload_triggered()
{
    if (!m_ctcTransfer)
        return;

    
    if (cobraNetHandler::instance()->isServing())
        QMessageBox::warning(this, tr("Server Warning"), tr("Server cannot download clips! Updating clip info only..."));
        
    QString filePath = g_cobra_settings->value("storage_dir").toString();
    QDir storage(filePath);
    if (!storage.exists()) {
        return;
    }

    debug(LOW, "Dowload Server Files!\n");
    QVector<int> serverList;

    ui->serverTree->getCheckedUidsDownload(serverList);

    debug(LOW, "Dowloading %d Clips \n", serverList.size());

    for (int x=0; x<serverList.size(); x++) {
        bool sendFlag = true;
        bool updateFlag = true;

        if (cobraNetHandler::instance()->isServing())
            sendFlag = false;

        filePath = g_cobra_settings->value("storage_dir").toString();
        cobraClip clip = ui->serverTree->getClip(serverList.at(x));

        if (clip.getUid() != serverList.at(x)) {
            debug(ERROR(CRITICAL), "Failed to find selected server clip!");
            continue;
        }

        QString hash = clip.getHash();
        cobraClip clipByHash = ui->serverTree->getClipByHash(hash);

        QString extension = clipByHash.getExtension();
        filePath += "/" + hash + "." + extension;

        cobraTransferFile* file = new cobraTransferFile(filePath);

        QByteArray currentHash = file->currentHash();
        if (currentHash != hash) {
            if (localLimitReached(clip.getSize())) {
                /* If the limit is reached, notify the user and
                  delete file; */
                QMessageBox::warning(this, tr("Size Limits"), tr("Local size limit reached!\n(See 'Preferences->Limits->Local Limits'\n"
                                                                 "to change the on disk size limit."));
                return;
            }
        }
        else {
            if (!cobraNetHandler::instance()->isServing()) {
                QMessageBox::warning(this, tr("File Exists"), tr("File already exists in the local area..."));
                sendFlag = false;
            }
        }

            file->setExpectedHash(hash.toUtf8());
            file->setDestination(cobraMyId);
            file->setSource(SERVER);
            file->setSending(false);
            m_ctcTransfer->addTransfer(file);

            cobraClipUpdateEvent* event = new cobraClipUpdateEvent();

            event->setSource(cobraMyId);
            event->setDestination(SERVER);
            event->setCommand(cobraClipUpdateEvent::FileRequest);
            event->setClip(clipByHash);
            event->setResponse(false);

            cobraClipList *list = ui->localTree;
            cobraClipList *serverList = ui->serverTree;
            cobraClip serverClip = serverList->getClipByHash(hash);

            cobraClip localClip = serverClip;
            localClip.setPath(filePath);

            if (list->containsHash(hash)) {
                if( QMessageBox::question(this, tr("Unable to Add Clip"), tr("Clip already exists! Would you like to overwrite the existing entry?"), QMessageBox::Yes,QMessageBox::No,QMessageBox::NoButton) == QMessageBox::Yes )
                {
                    debug(ERROR(LOW), "Overwritting the clip in the local list\n");
                }
                else
                {
                    debug(ERROR(LOW), "Not overwritting the clip in the local list\n");
                    updateFlag = false;
                }
            }

            if (updateFlag == true)
                list->addClip(localClip);

            if (sendFlag == true)
                cobraSendEvent(event);

            ui->serverTree->setDownload(serverClip.getUid(), false);

            notifyTransferComplete(localClip.getHash());
        }

        debug(LOW, "Done Downloading Files!\n");
}

void
MainWindow::notifyTransferComplete(QString hash)
{
    cobraClip clip = ui->localTree->getClipByHash(hash);
    if (clip.getUid() <= 0) {
        clip = ui->serverTree->getClipByHash(hash);
        if (clip.getUid() <= 0) {
            debug(ERROR(CRITICAL), "Invalid Hash for complete Transfer!\n");
            return;
        }
    }
    QString prompt = "Transfer Complete";
    statusBar()->showMessage(QString("%1: %2").arg(prompt).arg(clip.getTitle()));
}

void
MainWindow::on_actionSync_triggered()
{
    bool wantUpload = false;
    bool wantDownload = false;

    if (cobraNetHandler::instance()->isConnected() == true) {
        QMessageBox msgBox;
        msgBox.setText(tr("You have selected to sync with the server.  "
                          "Please select whether you wish to upload and/or download Clip Information."));
        QAbstractButton *cancelButton = msgBox.addButton(tr("     Cancel     "), QMessageBox::YesRole);
        QAbstractButton *bothButton = msgBox.addButton(tr(  "    Sync Both   "), QMessageBox::YesRole);
        QAbstractButton *serverButton = msgBox.addButton(tr("Download Changes"), QMessageBox::YesRole);
        QAbstractButton *localButton = msgBox.addButton(tr( " Upload Changes "), QMessageBox::YesRole);
        msgBox.setIcon(QMessageBox::Question);
        msgBox.exec();

        if(msgBox.clickedButton() == cancelButton) {
            debug(ERROR(LOW), "Canceled Sync List\n");
        }
        else if(msgBox.clickedButton() == bothButton) {
            debug(ERROR(LOW), "Sync Both Lists\n");
            wantUpload = true;
            wantDownload = true;
        }
        else if(msgBox.clickedButton() == serverButton) {
            debug(ERROR(LOW), "Sync From Server List\n");
            wantDownload = true;
        }
        else if(msgBox.clickedButton() == localButton) {
            debug(ERROR(LOW), "Sync From Local List\n");
            wantUpload = true;
        }

        if (wantUpload)
            sendLocalUpdates();

        if (wantDownload)
            refreshServerList();
    } else {
        QMessageBox msgBox;
        msgBox.setText(tr("Please Start or Connect to a server before attempting list sync."));
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();
    }
}

void
MainWindow::sendLocalUpdates()
{
    debug(LOW, "Send local updates!\n");
    QVector<int> localList;
    QVector<int> serverList;

    ui->localTree->getCheckedUids(localList);
    ui->serverTree->getCheckedUids(serverList);

    for (int x=0; x<serverList.size(); x++) {
        cobraClip clip = ui->serverTree->getClip(serverList.at(x));
        if (clip.getUid() != serverList.at(x)) {
            debug(ERROR(CRITICAL), "Failed to find selected server clip!");
            continue;
        }

        cobraClipUpdateEvent* event = new cobraClipUpdateEvent();
        event->setSource(cobraMyId);
        event->setDestination(SERVER);
        event->setResponse(false);
        event->setClip(clip);
        event->setCommand(cobraClipUpdateEvent::Update);

        cobraSendEvent(event);
    }

    for (int x=0; x<localList.size(); x++)
    {
        debug(CRITICAL, "Processing UID: %d\n", localList.at(x));
        cobraClip clip = ui->localTree->getClip(localList.at(x));
        if (clip.getUid() != localList.at(x)) {
            debug(ERROR(CRITICAL), "Failed to find selected server clip!");
            continue;
        }

        cobraClipUpdateEvent* event = new cobraClipUpdateEvent();
        event->setSource(cobraMyId);
        event->setDestination(SERVER);
        event->setResponse(false);
        event->setClip(clip);
        event->setCommand(cobraClipUpdateEvent::Add);

        cobraSendEvent(event);

        QString path = clip.getPath();
        cobraTransferFile* file = new cobraTransferFile(path);
        file->setDestination(SERVER);
        file->setSource(cobraMyId);
        file->setSending(true);

        cobraTransferEvent* tevent = new cobraTransferEvent;
        tevent->fromFile(file);
        tevent->setResponse(false);
        tevent->setExtension(path);

        cobraTransferController::addPendingTransfer(file);
        cobraSendEvent(tevent);
    }

    ui->serverTree->synchronized();
    ui->localTree->synchronized();
}

void
MainWindow::refreshServerList()
{
    cobraClipUpdateEvent* event = new cobraClipUpdateEvent();
    event->setSource(cobraMyId);
    event->setDestination(SERVER);
    event->setCommand(cobraClipUpdateEvent::RequestSync);

    if (!cobraNetHandler::instance()->isServing()) {
        cobraClipList* hack = dynamic_cast<cobraClipList*>(ui->serverTree);

        if (!hack)
            return;

        debug(ERROR(CRITICAL), "Synchorinizing!\n");

        QVector<int> list;
        hack->enumClips(list);

        for (int x=0; x<list.size(); x++) {
            hack->removeClip(list.at(x));
        }
    }

    cobraSendEvent(event);
}

void
MainWindow::on_actionAway_triggered()
{
    sendAwayStatus();
}

void
MainWindow::on_actionOnline_triggered()
{
    sendOnlineStatus();
}

bool
MainWindow::sendAwayStatus()
{
    cobraChatEvent* chat = NULL;

    if (!cobraNetHandler::instance()->isConnected()) {
        debug(ERROR(HIGH), "Attempted to update status while not connected\n");
        ui->chatText->append(CHAT_NOTIFY("Failed to set status: Not Connected."));
        return false;
    }
    QString text;
    text.append(CHAT_AWAY("IS AWAY...!"));

    chat = new cobraChatEvent();
    if (!chat)
        return false;

    chat->setDestination(SERVER);
    chat->setSource(cobraMyId);

    chat->setMsg(text);
    cobraSendEvent(chat);

    return true;
}

bool
MainWindow::sendOnlineStatus()
{
    cobraChatEvent* chat = NULL;

    if (!cobraNetHandler::instance()->isConnected()) {
        debug(ERROR(HIGH), "Attempted to update status while not connected\n");
        ui->chatText->append(CHAT_NOTIFY("Failed to set status: Not Connected."));
        return false;
    }

    QString text;
    text.append(CHAT_ONLINE("IS BACK ONLINE...!"));

    chat = new cobraChatEvent();
    if (!chat)
        return false;

    chat->setDestination(SERVER);
    chat->setSource(cobraMyId);

    chat->setMsg(text);
    cobraSendEvent(chat);

    return true;
}

void
MainWindow::on_actionMarker_Toolbar_triggered(bool checked)
{
    ui->TimelineList->setVisible(checked);
    checked =!checked;
}

bool
MainWindow::updateLocalListDb(QString db)
{
    QString old = ui->localTree->getName();
    if (!ui->localTree->setName(db)) {
        QMessageBox::critical(this, tr("Invalid Database!"), tr("The specified Local List file could not opened or is invalid!"));
        ui->localTree->setName(old);
        g_cobra_settings->setValue("db/local", old);
        return false;
    }
    return true;
}

bool
MainWindow::updateTimelineDb(QString db)
{
    QString old = ui->timelineTree->getName();
    if (!ui->timelineTree->setName(db)) {
        QMessageBox::critical(this, tr("Invalid Database!"), tr("The specified Timeline file could not opened or is invalid!"));
        ui->timelineTree->setName(old);
        g_cobra_settings->setValue("db/timeline", old);
        return false;
    }
    return true;
}

bool
MainWindow::updateServerDb(QString db)
{
    QString old = ui->serverTree->getName();
    if (!ui->serverTree->setName(db)) {
        QMessageBox::critical(this, tr("Invalid Database!"), tr("The specified Server file could not opened or is invalid!"));
        ui->serverTree->setName(old);
        g_cobra_settings->setValue("db/server", old);
        return false;
    }
    return true;
}


void
MainWindow::on_actionAddMark_triggered()
{
    if (!m_mDialog)
        m_mDialog = new markerDialog();

    cobraTimeline timeline = ui->timelineTree->getCurrentTimeline();
    if (timeline.getUid() <= 0) {
        QMessageBox::warning(this, tr("No Timeline!"), tr("Please add a Timeline prior to attempting to add a Mark!"));
        return;
    }

    m_mDialog->setComment("");
    m_mDialog->exec();

    if (m_mDialog->result()) {
        cobraMark mark;
        mark.setComment(m_mDialog->getComment());
        cobraTimelineEvent* event = new cobraTimelineEvent;
        event->setMark(mark);
        event->setCommand(cobraTimelineEvent::GenerateMark);
        cobraSendEvent(event);
    }
}

void
MainWindow::on_actionAddTimeline_triggered()
{
    if (!m_tlDialog)
        m_tlDialog = new cobraTimelineDialog();

    m_tlDialog->setTitle("");
    m_tlDialog->setDescription("");

    cobraTimeline timeline;
    do {
        m_tlDialog->exec();
        if (!m_tlDialog->result())
            return;
        timeline.setTitle(m_tlDialog->getTitle());
        if (timeline.getTitle().isEmpty() || timeline.getTitle().isNull() || !timeline.getTitle().length()) {
            QMessageBox::warning(this, tr("Title Required!"), tr("A title must be specified."));
            continue;
        }
        break;
    } while(1);

    timeline.setDescription(m_tlDialog->getDescription());

    cobraTimelineEvent* event = new cobraTimelineEvent;
    event->setTimeline(timeline);
    event->setCommand(cobraTimelineEvent::GenerateTimeline);
    cobraSendEvent(event);
}

void
MainWindow::on_actionRemove_Timeline_triggered()
{
    QMessageBox msgBox;
    msgBox.setText(tr("Are you sure you want to remove the selected timeline?"));
    msgBox.setWindowTitle(tr("Warning"));
    msgBox.setModal(true);

    QPushButton *noButton = msgBox.addButton(tr("     No     "), QMessageBox::YesRole);
    QPushButton *yesButton = msgBox.addButton(tr(  "    Yes   "), QMessageBox::YesRole);

    msgBox.setDefaultButton(noButton);
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.exec();

    if(msgBox.clickedButton() == noButton) {
        debug(ERROR(LOW), "Canceled timeline removal\n");
        msgBox.accept();
    }
    else if(msgBox.clickedButton() == yesButton) {
        debug(ERROR(LOW), "Timeline removal confirmed\n");
        QTreeWidgetItem* ci = ui->timelineTree->currentItem();
        if (ci!=NULL) {
            int uid = ci->text(cobraTimelineTree::TID).toInt(0,10);
            ui->timelineTree->removeTimeline(uid);
        } else return;
    }
}

void
MainWindow::on_actionExportFileList_triggered()
{
    QVector<int> cl;

    QString path = QFileDialog::getSaveFileName(this, tr("Export clip list to..."), QString::null);

    if (path.isEmpty())
        return; //canceled
    else QFile::remove(path);

    cobraClipList* exportList = new cobraClipList(path);
    ui->localTree->enumClips(cl);

    for(QVector<int>::iterator itr=cl.begin(); itr != cl.end(); itr++) {
        cobraClip clip = ui->localTree->getClip(*itr);
        exportList->addClip(clip);
    }

}

void
MainWindow::on_actionExportTimelineList_triggered()
{
    QVector<int> timelinelist, markerlist;

    QString path = QFileDialog::getSaveFileName(this, tr("Export Timeline to..."), QString::null);

    if (path.isEmpty())
        return; //canceled
    else QFile::remove(path);

    cobraTimelineList* exportList = new cobraTimelineList(path);
    ui->timelineTree->enumTimelines(timelinelist);

    exportList->setName(path);

    for(QVector<int>::iterator itr=timelinelist.begin(); itr != timelinelist.end(); itr++) {
        cobraTimeline timeline = ui->timelineTree->getTimeline(*itr);
        ui->timelineTree->enumTimelineMarkers(markerlist, timeline);
        //export markers
        for(QVector<int>::iterator itrm=markerlist.begin(); itrm != markerlist.end(); itrm++){
            cobraMark marker = ui->timelineTree->getMark(*itrm);
            exportList->addMark(marker);
        }
        exportList->addTimeline(timeline);
    }
}

void
MainWindow::on_actionMute_triggered(bool checked)
{
    if (checked)
        ui->actionMute->setIcon(QIcon(":images/mute.png"));
    else
        ui->actionMute->setIcon(QIcon(":images/audio.png"));

#if !defined(NO_AUDIO)
    if (m_aoAudioOutput)
        m_aoAudioOutput->setMuted(checked);
#endif
}

void
MainWindow::on_actionPlay_triggered()
{
    if (!m_cclFocused)
        return;

    cobraClipList* list = m_cclFocused;
    cobraTreeWidget* tree = dynamic_cast<cobraTreeWidget*>(m_cclFocused);
    if (!tree)
        return;

    QTreeWidgetItem* item = tree->currentItem();
    int uid = item->text(cobraTreeWidget::ID).toInt();
    cobraClip clip = list->getClip(uid);

    if (tree == ui->serverTree)
        playRemote(clip);
    else
        playLocal(clip);
}

void
MainWindow::playRemote(const cobraClip& clip)
{
    cobraPlayEvent* event = new cobraPlayEvent;

    event->setHash(clip.getHash());
    event->setDestination(SERVER);
    event->setSource(cobraMyId);
    event->setResponse(false);

    cobraSendEvent(event);
}

void
MainWindow::playServerHash(const QString& hash)
{
    cobraClip clip = ui->serverTree->getClipByHash(hash);
    if (clip.getUid() <= 0)
        return;

    playLocal(clip);
}

void
MainWindow::playLocal(const cobraClip& clip)
{
    QString path = clip.getPath();
    QFile file(path);
    if (!file.exists()) {
        path = g_cobra_settings->value("storage_dir").toString();
        path += "/" + clip.getHash() + "." + clip.getExtension();
        QFile constructedFile(path);
        if (!constructedFile.exists()) {
            QMessageBox::warning(this, "Play Audio", "Failed to find selected audio file!");
            return;
        }
    }

#if !defined(NO_AUDIO)
    if (!m_aoAudioOutput) {
        m_aoAudioOutput = new Phonon::AudioOutput(Phonon::MusicCategory, this);
        m_moAudioFile = new Phonon::MediaObject(this);
        Phonon::Path path = Phonon::createPath(m_moAudioFile, m_aoAudioOutput);
        debug(HIGH, "Is Valid: %d\n", path.isValid());

        m_aoAudioOutput->setVolume(6.0);
    }

    m_aoAudioOutput->setMuted(ui->actionMute->isChecked());

    m_moAudioFile->setCurrentSource(path);
    m_moAudioFile->play();
    debug(HIGH, "Playing Audio Clip: %s\n", qPrintable(path));
#else
    QMessageBox::warning(this, "Play Audio", "Would have played:" + path);
#endif
}
