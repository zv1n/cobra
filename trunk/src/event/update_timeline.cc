/*******************************************************************************
Copyright (c) 2012, All rights reserved.
	Terry Meacham <terry.t.meacham@gmail.com>
	Andrew Hicks <andrew.t.hicks@gmail.com>
	Nick Huskinson <nhuskinson@gmail.com>
	Jiake Liu  <jiakeliu@gmail.com>
*******************************************************************************/

#include "debug.h"
#include "net.h"

#include <QApplication>
#include <QMessageBox>
#include <QObject>

#include "timeline.h"

cobraTimelineEvent::cobraTimelineEvent()
    :cobraNetEvent(cobraTimelineEventType)
{}

cobraTimelineEvent::cobraTimelineEvent(cobraTimelineEvent& event)
    :cobraNetEvent(event), m_iCommand(event.m_iCommand),
    m_ctTimeline(event.m_ctTimeline), m_cmMark(event.m_cmMark)
{}

cobraTimelineEvent::~cobraTimelineEvent()
{}

int cobraTimelineEvent::serialize(QDataStream& connection)
{
    int size = cobraNetEvent::serialize(connection);

    connection << m_iCommand;
    size += sizeof(m_iCommand);

    if (m_iCommand == cobraTimelineEvent::GenerateTimeline) {
        debug(CRITICAL, "Serializing Timeline Packet! %s:%s\n",
          qPrintable(QString::number(m_ctTimeline.getUid())), qPrintable(m_ctTimeline.getTitle()));

        connection << QString::number(m_ctTimeline.getUid());
        connection << m_ctTimeline.getTitle();
        connection << m_ctTimeline.getDescription();
        connection << m_ctTimeline.getStartTime().toString("yyyy-MM-dd HH:mm:ss");

        return size + QString::number(m_ctTimeline.getUid()).length() +
                  m_ctTimeline.getTitle().length() +
                  m_ctTimeline.getDescription().length() +
                  m_ctTimeline.getStartTime().toString("yyyy-MM-dd HH:mm:ss").length();
    }
    else if (m_iCommand == cobraTimelineEvent::GenerateMark) {
        debug(CRITICAL, "Serializing Mark Packet! %s:%s\n",
          qPrintable(QString::number(m_cmMark.getUid())), qPrintable(QString::number(m_cmMark.getTimelineKey())));

        connection << QString::number(m_cmMark.getUid());
        connection << QString::number(m_cmMark.getTimelineKey());
        connection << m_cmMark.getComment();
        connection << QString::number(m_cmMark.getTimeOffset());
    
        return size + QString::number(m_cmMark.getUid()).length() +
                  QString::number(m_cmMark.getTimelineKey()).length() +
                  m_cmMark.getComment().length() +
                  QString::number(m_cmMark.getTimeOffset()).length();
 
    }
    else {
        debug(ERROR(CRITICAL), "m_iCommand should be GenrateTimeline or GenerateMark\n");
        return -1;
    }  
}

int
cobraTimelineEvent::deserialize(QDataStream& connection)
{
    int size = cobraNetEvent::deserialize(connection);
    QString tmpString;

    connection >> m_iCommand;
    size += sizeof(m_iCommand);

    if (m_iCommand == cobraTimelineEvent::GenerateTimeline) {
        connection >> tmpString;
        size += tmpString.length();
        m_ctTimeline.setUid(tmpString.toInt());
    
        connection >> tmpString;
        size += tmpString.length();
        m_ctTimeline.setTitle(tmpString);
    
        connection >> tmpString;
        size += tmpString.length();
        m_ctTimeline.setDescription(tmpString);
    
        connection >> tmpString;
        size += tmpString.length();
        m_ctTimeline.setStartTime(tmpString);

        return size;
    }
    else if (m_iCommand == cobraTimelineEvent::GenerateMark) {
        connection >> tmpString;
        size += tmpString.length();
        m_cmMark.setUid(tmpString.toInt());
    
        connection >> tmpString;
        size += tmpString.length();
        m_cmMark.setTimelineKey(tmpString.toInt());
    
        connection >> tmpString;
        size += tmpString.length();
        m_cmMark.setComment(tmpString);
    
        connection >> tmpString;
        size += tmpString.length();
        m_cmMark.setTimeOffset(tmpString.toInt());
    
        return size;
    }
    else {
        debug(ERROR(CRITICAL), "m_iCommand should be GenrateTimeline or GenerateMark\n");
        return -1;
    }
}

cobraTimeline
cobraTimelineEvent::timeline() const
{
    return m_ctTimeline;
}

void
cobraTimelineEvent::setTimeline(const cobraTimeline &newTimeline)
{
    m_ctTimeline = newTimeline;
}

cobraMark
cobraTimelineEvent::mark() const
{
    return m_cmMark;
}

void
cobraTimelineEvent::setMark(const cobraMark &newMark)
{
    m_cmMark = newMark;
}

int
cobraTimelineEvent::command() const
{
    return m_iCommand;
}

void
cobraTimelineEvent::setCommand(int command)
{
    m_iCommand = command;
}

cobraNetEvent*
cobraTimelineEvent::duplicate()
{
    cobraTimelineEvent* timelineDupe = new cobraTimelineEvent(*this);
    return timelineDupe;
}


cobraTimelineEventHandler::cobraTimelineEventHandler()
    :cobraNetEventHandler("TimelineUpdate", cobraTimelineEventType)
{}

cobraTimelineEventHandler::cobraTimelineEventHandler(cobraTimelineEventHandler& event)
    :cobraNetEventHandler(event)
{}

cobraTimelineEventHandler::~cobraTimelineEventHandler()
{}

void
cobraTimelineEventHandler::setTimelineList(cobraTimelineList* list)
{
    m_ctlTimeline = list;
}

void
cobraTimelineEventHandler::setLastTimeline(QDateTime tl)
{
    m_dtLastTimeline = tl;
}

bool
cobraTimelineEventHandler::handleEvent(cobraNetEvent* event)
{
    if (!event)
        return false;

    if (event->type() != cobraTimelineEventType)
        return false;

    if (event->isRequest())
        return handleServerEvent(event);

    cobraTimelineEvent* tup = static_cast<cobraTimelineEvent*>(event);
    debug(ERROR(CRITICAL), "Handling Timeline Event: Client! %d\n", tup->command());

    switch(tup->command()) {
    case cobraTimelineEvent::GenerateMark:
        clientMark(tup);
        break;

    case cobraTimelineEvent::GenerateTimeline:
        clientTimeline(tup);
        break;

    case cobraTimelineEvent::Reject:
        QMessageBox::warning(NULL, QObject::tr("Timeline Request Reject"),
                             QObject::tr("The requested timeline event was reject by the server!\n"));
        break;

    default:
        debug(ERROR(CRITICAL), "Default!!!");
        break;
    }

    return true;
}

bool
cobraTimelineEventHandler::handleServerEvent(cobraNetEvent* event)
{

    cobraTimelineEvent* tup = static_cast<cobraTimelineEvent*>(event);

    debug(ERROR(CRITICAL), "Handling Timeline Event: Server! %d\n", tup->command());
    if (cobraNetHandler::instance()->getIdAuthorization(event->source()) != ParticipantAuth) {
        cobraTimelineEvent* revent = new cobraTimelineEvent;
        revent->setDestination(event->source());
        revent->setSource(event->destination());
        revent->setResponse(true);
        revent->setCommand(cobraTimelineEvent::Reject);
        return cobraNetHandler::instance()->sendEvent(revent);
    }

    switch(tup->command()) {
    case cobraTimelineEvent::GenerateMark:
        serverMark(tup);
        break;

    case cobraTimelineEvent::GenerateTimeline:
        serverTimeline(tup);
        break;

    default:
        debug(ERROR(CRITICAL), "Default!!!");
        break;
    }

    return true;
}

void
cobraTimelineEventHandler::clientMark(cobraTimelineEvent* tup)
{
    if (!m_ctlTimeline)
        return;

    debug(ERROR(CRITICAL), "Client Mark!!!\n");
    cobraMark mark = tup->mark();
    cobraTimeline timeline = m_ctlTimeline->getCurrentTimeline();
    mark.setTimelineKey(timeline.getUid());
    m_ctlTimeline->addMark(mark);
}

void
cobraTimelineEventHandler::clientTimeline(cobraTimelineEvent* tup)
{
    if (!m_ctlTimeline)
        return;

    debug(ERROR(CRITICAL), "Client Timeline!!!\n");
    cobraTimeline timeline = tup->timeline();
    m_ctlTimeline->addTimeline(timeline);
}

void
cobraTimelineEventHandler::serverMark(cobraTimelineEvent* fmark)
{
    debug(ERROR(CRITICAL), "Server Mark!!!\n");
    cobraTimelineEvent* tup = static_cast<cobraTimelineEvent*>(fmark->duplicate());

    cobraMark mark = tup->mark();
    mark.setTimeOffset(m_dtLastTimeline);
    tup->setMark(mark);

    tup->setSource(SERVER);
    tup->setDestination(BROADCAST);
    tup->setResponse(true);
    cobraSendEvent(tup);
}

void
cobraTimelineEventHandler::serverTimeline(cobraTimelineEvent* fline)
{
    debug(ERROR(CRITICAL), "Server Timeline!!!\n");
    cobraTimelineEvent* tup = static_cast<cobraTimelineEvent*>(fline->duplicate());

    cobraTimeline timeline = tup->timeline();
    timeline.setStartTime();
    m_dtLastTimeline = timeline.getStartTime();

    tup->setTimeline(timeline);
    tup->setSource(SERVER);
    tup->setDestination(BROADCAST);
    tup->setResponse(true);
    cobraSendEvent(tup);
}

cobraNetEvent*
cobraTimelineEventHandler::eventGenesis() const
{
    return new cobraTimelineEvent();
}

