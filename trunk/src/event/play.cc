/*******************************************************************************
Copyright (c) 2012, All rights reserved.
	Terry Meacham <terry.t.meacham@gmail.com>
	Andrew Hicks <andrew.t.hicks@gmail.com>
	Nick Huskinson <nhuskinson@gmail.com>
	Jiake Liu  <jiakeliu@gmail.com>
*******************************************************************************/

#include "debug.h"
#include "net.h"

cobraPlayEvent::cobraPlayEvent()
    :cobraNetEvent(cobraPlayEventType), m_iCommand(cobraPlayEvent::Play)
{}

cobraPlayEvent::cobraPlayEvent(cobraPlayEvent& play)
    :cobraNetEvent(play), m_iCommand(play.m_iCommand),
      m_sHash(play.m_sHash)
{}

cobraPlayEvent::~cobraPlayEvent()
{}

int
cobraPlayEvent::command() const
{
  return m_iCommand;
}

QString
cobraPlayEvent::hash() const
{
  return m_sHash;
}

void
cobraPlayEvent::setCommand(int cmd)
{
  m_iCommand = cmd;
}

void
cobraPlayEvent::setHash(QString hash)
{
  m_sHash = hash;
}

int
cobraPlayEvent::serialize(QDataStream& connection)
{
    int size = cobraNetEvent::serialize(connection);
    connection << m_iCommand;
    connection << m_sHash;
    debug(CRITICAL, "Serialize Play: %d\n", m_iCommand);
    return (size + m_sHash.size()+ sizeof(m_iCommand));
}

int
cobraPlayEvent::deserialize(QDataStream& connection)
{
    int size = cobraNetEvent::deserialize(connection);
    connection >> m_iCommand;
    connection >> m_sHash;
    debug(CRITICAL, "Deserialize Play: %d\n", m_iCommand);
    return (size + m_sHash.size()+ sizeof(m_iCommand));
}

cobraNetEvent*
cobraPlayEvent::duplicate()
{
    cobraPlayEvent* auth = new cobraPlayEvent(*this);
    return auth;
}

cobraPlayEventHandler::cobraPlayEventHandler()
    :cobraNetEventHandler("Play", cobraPlayEventType)
{}

cobraPlayEventHandler::cobraPlayEventHandler(cobraPlayEventHandler& event)
    :cobraNetEventHandler(event)
{}

cobraPlayEventHandler::~cobraPlayEventHandler()
{}

bool
cobraPlayEventHandler::handleEvent(cobraNetEvent* event)
{
    if (event->isRequest())
        return handleServerEvent(event);

    cobraPlayEvent* play = static_cast<cobraPlayEvent*>(event);
    debug(CRITICAL, "Recieved Server Play Event: %lu\n", (unsigned long)QThread::currentThreadId());

    switch(play->command())
    {
    case cobraPlayEvent::Play:
        {
            break;
        }

    case cobraPlayEvent::Reject:
        {
            break;
        }

    }

    return true;
}

bool
cobraPlayEventHandler::handleServerEvent(cobraNetEvent* event)
{
    if (!event)
        return false;

    if (event->type() != cobraPlayEventType)
        return false;

    debug(CRITICAL, "Handling Client Play Event: %lu\n", (unsigned long)QThread::currentThreadId());

    cobraPlayEvent* play = dynamic_cast<cobraPlayEvent*>(event);
    if (!play)
        return false;

    if (m_mwMain)
        m_mwMain->playServerHash(play->hash());
    return true;
}

cobraNetEvent*
cobraPlayEventHandler::eventGenesis() const
{
    return new cobraPlayEvent();
}

void
cobraPlayEventHandler::setWindow(MainWindow *window)
{
    m_mwMain = window;
}

MainWindow*
cobraPlayEventHandler::window() const
{
    return m_mwMain;
}
