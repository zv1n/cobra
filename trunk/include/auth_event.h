/*******************************************************************************
Copyright (c) 2012, All rights reserved.
	Terry Meacham <terry.t.meacham@gmail.com>
	Andrew Hicks <andrew.t.hicks@gmail.com>
	Nick Huskinson <nhuskinson@gmail.com>
	Jiake Liu  <jiakeliu@gmail.com>
*******************************************************************************/

#ifndef AUTH_EVENT_H
#define AUTH_EVENT_H

#include "event.h"

class cobraAuthEvent;
class cobraAuthEventHandler;

/**
 * @class cobraAuthEventHandler event.h "event.h"
 *
 * The cobraAuthEventHandler is used to handle incoming authorization requests.
 */
class cobraAuthEventHandler : public cobraNetEventHandler {

public:
    cobraAuthEventHandler();
    cobraAuthEventHandler(cobraAuthEventHandler& event);
    virtual ~cobraAuthEventHandler();

public:
    virtual bool handleEvent(cobraNetEvent* event);

    /**
     * @fn virtual cobraNetEvent* eventGenesis() const;
     * Generates an event of the type that it handles.
     */
    virtual cobraNetEvent* eventGenesis() const;

protected:
    virtual bool handleServerEvent(cobraNetEvent* event);
};

/**
 * @class cobraAuthEvent event.h "event.h"
 *
 * The event used to communicate authentication information to the server and other
 * machines.
 */

class cobraAuthEvent : public cobraNetEvent {
public:
   cobraAuthEvent();
   cobraAuthEvent(cobraAuthEvent& state);
   virtual ~cobraAuthEvent();

   QString username() const;
   void setUsername(const QString& user);

   QString password() const;
   void setPassword(const QString& pwd);

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
protected:
   QString m_sUsername;
   QString m_sPassword;
};

#endif // AUTH_EVENT_H
