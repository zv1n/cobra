/*******************************************************************************
Copyright (c) 2012, All rights reserved.
	Terry Meacham <terry.t.meacham@gmail.com>
	Andrew Hicks <andrew.t.hicks@gmail.com>
	Nick Huskinson <nhuskinson@gmail.com>
	Jiake Liu  <jiakeliu@gmail.com>
*******************************************************************************/

#ifndef CHAT_EVENT_H
#define CHAT_EVENT_H

#include "event.h"

class cobraChatEvent;
class cobraChatEventHandler;

/**
 * @class cobraChatEvent event.h "event.h"
 *
 * The event used to communicate chat information to the server and other
 * machines.
 */

class cobraChatEvent : public cobraNetEvent {
public:
   cobraChatEvent();
   cobraChatEvent(cobraChatEvent& state);
   virtual ~cobraChatEvent();

   QString msg() const;
   void setMsg(const QString& msg);

   int command() const;
   void setCommand(int cmd);

public:

   /**
    * @fn virtual int serialize(QDataStream& stream)
    * Serialize the data for chats which is pending on the given cobraNetConnection.
    */
   virtual int serialize(QDataStream& stream);

   /**
    * @fn virtual int deserialize(QDataStream& stream)
    * Deserialize the data for chats which is pending on the given cobraNetConnection.
    */
   virtual int deserialize(QDataStream& stream);

   /**
    * @fn virtual cobraNetEvent* duplicate()
    * Create a new duplicate event and dump all values to it!
    * This is called to ensure that the main event loop or thread loop
    * doesn't delete it while we are using it, as once posted, the event loop
    * handles deletion.
    * @return A pointer to the copied cobra event.
    */
   virtual cobraNetEvent* duplicate();

   enum ChatCommands {
       ChatMessage,
       ChangeUsername,
       Away,
       ListUpdate
   };

protected:
    int m_iCommand;
    QString m_sMessage;
};

/**
 * @class cobraChatEventHandler event.h "event.h"
 *
 * The cobraChatEventHandler is used to handle incoming chat events.
 * Because this event handler interfaces with the User Interface, it
 * absolutly CAN NOT use threads to process incoming information.
 */
class cobraChatEventHandler : public cobraNetEventHandler {

public:
    cobraChatEventHandler();
    cobraChatEventHandler(cobraChatEventHandler& event);
    virtual ~cobraChatEventHandler();

public:
    virtual bool handleEvent(cobraNetEvent* event);

    /**
     * @fn virtual cobraNetEvent* eventGenesis() const;
     * Generates an event of the type that it handles.
     */
    virtual cobraNetEvent* eventGenesis() const;

    bool            setChatDock(QDockWidget* chat);

protected:
    virtual bool handleServerEvent(cobraNetEvent* event);

protected:
    QDockWidget*    m_dwChatDock;
    QPushButton*    m_pbSend;
    QTextEdit*      m_teText;
    QTextEdit*      m_teChat;
    QListWidget*    m_lwUserlist;
};

#endif // CHAT_EVENT_H
