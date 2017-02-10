/*******************************************************************************
Copyright (c) 2012, All rights reserved.
	Terry Meacham <terry.t.meacham@gmail.com>
	Andrew Hicks <andrew.t.hicks@gmail.com>
	Nick Huskinson <nhuskinson@gmail.com>
	Jiake Liu  <jiakeliu@gmail.com>
*******************************************************************************/

#include "net.h"
#include "debug.h"
#include "mainwindow.h"
#include <QMessageBox>

cobraTransferEvent::cobraTransferEvent()
    :cobraNetEvent(cobraTransferEventType),
      m_iCommand(cobraTransferEvent::Request),
      m_iOffset(0), m_iSize(0)
{
}

cobraTransferEvent::cobraTransferEvent(cobraTransferEvent& state)
    :cobraNetEvent(state), m_iCommand(state.m_iCommand),
      m_iOffset(state.m_iOffset), m_iSize(state.m_iSize),
      m_baHash(state.m_baHash), m_baData(state.m_baData), m_sExtension(state.m_sExtension)
{
}

cobraTransferEvent::~cobraTransferEvent()
{
}

int
cobraTransferEvent::serialize(QDataStream& stream)
{
    int bytes = cobraNetEvent::serialize(stream);

    stream << m_iCommand;
    stream << m_iOffset;
    stream << m_iSize;
    stream << m_baHash;
    stream << m_baData;
    stream << m_sExtension;

    return (bytes + sizeof(m_iOffset) +
            sizeof(m_iCommand) + m_baData.length() +
            sizeof(m_iSize) + sizeof(m_iOffset) + m_baHash.length());
}

int
cobraTransferEvent::deserialize(QDataStream& stream)
{
    int bytes = cobraNetEvent::deserialize(stream);

    stream >> m_iCommand;
    stream >> m_iOffset;
    stream >> m_iSize;
    stream >> m_baHash;
    stream >> m_baData;
    stream >> m_sExtension;

    return (bytes + sizeof(m_iOffset) +
            sizeof(m_iCommand) + m_baData.length() +
            sizeof(m_iSize) + sizeof(m_iOffset) + m_baHash.length());
}

cobraNetEvent*
cobraTransferEvent::duplicate()
{
    cobraTransferEvent* xfer = new cobraTransferEvent(*this);
    return xfer;
}

QString cobraTransferEvent::extension()
{
    return m_sExtension;
}

void
cobraTransferEvent::setExtension(QString fileName)
{
    int index = fileName.lastIndexOf(".");
    if (index < 0)
        m_sExtension = fileName;
    else
        m_sExtension = fileName.remove(0, index+1);
}

bool
cobraTransferEvent::fromFile(cobraTransferFile* file)
{
    if (!file)
        return false;

    if (!file->exists())
        return false;

    m_baHash = file->hash();
    m_iOffset = 0;
    m_iSize = file->size();

    setDestination(file->destination());
    setSource(file->source());

    return true;
}

void
cobraTransferEvent::setCommand(int cmd)
{
    m_iCommand = cmd;
}

int
cobraTransferEvent::command() const
{
    return m_iCommand;
}

void
cobraTransferEvent::setOffset(qint64 offset)
{
    m_iOffset = offset;
}

qint64
cobraTransferEvent::offset() const
{
    return m_iOffset;
}

void
cobraTransferEvent::setSize(qint64 size)
{
    m_iSize = size;
}

qint64
cobraTransferEvent::size() const
{
    return m_iSize;
}

void
cobraTransferEvent::setData(QByteArray& data)
{
    m_baData = data;
}

void
cobraTransferEvent::clearData()
{
    m_baData.clear();
}

const QByteArray&
cobraTransferEvent::data() const
{
    return m_baData;
}

void
cobraTransferEvent::setHash(QByteArray& hash)
{
    m_baHash = hash;
}

const QByteArray&
cobraTransferEvent::hash() const
{
    return m_baHash;
}

cobraTransferEventHandler::cobraTransferEventHandler()
    :cobraNetEventHandler("Transfer", cobraTransferEventType),
      cobraTransferController(cobraConcurrentTransfers)
{
    initialize();
}

cobraTransferEventHandler::cobraTransferEventHandler(cobraTransferEventHandler& event)
    :cobraNetEventHandler(event), cobraTransferController(cobraConcurrentTransfers)
{
    initialize();
}

cobraTransferEventHandler::~cobraTransferEventHandler()
{
    cleanup();
}

bool
cobraTransferEventHandler::handleEvent(cobraNetEvent* event)
{
    debug(MED, "Transfer Handler!\n");
    bool ret = false;
    cobraNetHandler* handler = cobraNetHandler::instance();
    cobraTransferEvent* tevent = dynamic_cast<cobraTransferEvent*>(event);

    if (!tevent)
        return false;

    switch (tevent->command()) {

    case cobraTransferEvent::Chunk: {
        debug(LOW, "Transfer Chunk!\n");

        int cmp = cobraTransferController::recieveChunk(tevent);

        switch (cmp) {
        case cobraTransferFile::TransferComplete: {
            cobraTransferEvent* xevent = static_cast<cobraTransferEvent*>(event->duplicate());

            xevent->setCommand(cobraTransferEvent::Complete);
            xevent->setSource(event->destination());
            xevent->setDestination(event->source());
            xevent->setResponse((event->source()==SERVER));
            xevent->setOffset(0);
            xevent->setSize(0);
            xevent->clearData();

            debug(HIGH, "Transfer Complete: %s\n", qPrintable(QString(xevent->hash()).left(4)));

            //cobraSendEvent(xevent);
            handler->sendEvent(xevent);
            break;
        }

        case cobraTransferFile::TransferError: {
            debug(ERROR(CRITICAL), "Error while recieving chunk for file: %s\n",
                  qPrintable(QString(tevent->hash())));
            break;
        }
        }

        tevent->put();
        return ret;
    }

    case cobraTransferEvent::Request: {
        debug(LOW, "Transfer Request!\n");

        cobraTransferEvent* response = static_cast<cobraTransferEvent*>(tevent->duplicate());
        response->setDestination(tevent->source());
        response->setSource(tevent->destination());
        response->setCommand(cobraTransferEvent::Reject);
        response->setResponse(true);

        int auth = handler->getIdAuthorization(event->source());
        if (auth & ParticipantAuth) {
            if (m_mwMain && m_mwMain->serverLimitReached(tevent->size())) {
                /* If the limit is reached, notify the user and
                  delete file; */
                QMessageBox::warning(NULL, tr("Size Limits"),
                                     tr("Server size limit reached!\n"
                                     "(See 'Preferences->Limits->Server Limits'\n"
                                     "to change the on disk size limit."));
                return false;
              }

            QString filePath = g_cobra_settings->value("storage_dir").toString();

            QString extension = tevent->extension();
            QByteArray hash = tevent->hash();
            filePath += "/" + hash + "." + extension;

            cobraTransferFile* file = new cobraTransferFile(filePath);

            QByteArray currentHash = file->currentHash();
            if (currentHash == tevent->hash()) {
                delete file;
                response->setCommand(cobraTransferEvent::Exists);
                goto out;
            }

            file->setExpectedHash(hash);
            file->setDestination(tevent->destination());
            file->setSource(tevent->source());
            file->setSending(false);

            if (!addTransfer(file)) {
                debug(ERROR(CRITICAL), "Invalid file!\n");
                delete file;
            } else
                response->setCommand(cobraTransferEvent::Accept);
        }

out:
        handler->sendEvent(response);
        response->put();
        break;
    }

    case cobraTransferEvent::Accept: {
        debug(LOW, "Transfer Accept!\n");

        cobraTransferFile* file = cobraTransferController::getPendingTransfer(tevent->hash());
        if (!file) {
            debug(ERROR(CRITICAL), "Failed to find valid file!\n");
            break;
        }

        file->setDestination(event->source());
        file->setSource(event->destination());

        handler->sendFile(file);
        break;
    }

    case cobraTransferEvent::Reject:
        debug(LOW, "Transfer Rejected!\n");
        QMessageBox::warning(NULL, "Transfer Rejected",
                             "The server rejected your file transfer request!\n"
                             "You probably do not have adequite permissions!");
        break;

    case cobraTransferEvent::Exists: {
        cobraTransferFile* file = cobraTransferController::getPendingTransfer(tevent->hash());
        if (!file) {
            debug(ERROR(CRITICAL), "Failed to find valid file!\n");
            break;
        }

        delete file;
        break;
    }

    case cobraTransferEvent::Resend:
        debug(ERROR(HIGH), "This should be intercepted!\n");
        break;

    case cobraTransferEvent::Complete:
        m_mwMain->notifyTransferComplete(tevent->hash());
        debug(ERROR(HIGH), "Transfer Complete!\n");
        break;
    }

    tevent->put();
    return true;
}

bool
cobraTransferEventHandler::handleServerEvent(cobraNetEvent* event)
{
    debug(ERROR(CRITICAL), "Somehow we called into the cobraTransferEvent::handleServerEvent handler...\n");
    exit(1);
    (void)event;
    return false;
}

void
cobraTransferEventHandler::setWindow(MainWindow* main)
{
  m_mwMain = main;
}

cobraNetEvent*
cobraTransferEventHandler::eventGenesis() const
{
    return new cobraTransferEvent();
}

