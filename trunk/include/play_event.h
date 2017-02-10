/*******************************************************************************
Copyright (c) 2012, All rights reserved.
	Terry Meacham <terry.t.meacham@gmail.com>
	Andrew Hicks <andrew.t.hicks@gmail.com>
	Nick Huskinson <nhuskinson@gmail.com>
	Jiake Liu  <jiakeliu@gmail.com>
*******************************************************************************/

#ifndef PLAY_EVENT_H
#define PLAY_EVENT_H

#include "event.h"
#include "mainwindow.h"

class cobraPlayEvent;
class cobraPlayEventHandler;

/**
 * @class cobraPlayEventHandler event.h "event.h"
 *
 * The cobraPlayEventHandler is used to handle incoming play events.
 */
class cobraPlayEventHandler : public cobraNetEventHandler {

public:
    cobraPlayEventHandler();
    cobraPlayEventHandler(cobraPlayEventHandler& event);
    virtual ~cobraPlayEventHandler();

public:
    virtual bool handleEvent(cobraNetEvent* event);

    /**
     * @fn virtual cobraNetEvent* eventGenesis() const;
     * Generates an event of the type that it handles.
     */
    virtual cobraNetEvent* eventGenesis() const;

    void setWindow(MainWindow* window);
    MainWindow* window() const;

protected:
    virtual bool handleServerEvent(cobraNetEvent* event);

protected:
    MainWindow* m_mwMain;
};


/**
 * @class cobraPlayEvent event.h "event.h"
 *
 * The event used to communicate authentication information to the server and other
 * machines.
 */

class cobraPlayEvent: public cobraNetEvent {
public:
   cobraPlayEvent();
   cobraPlayEvent(cobraPlayEvent& play);
   virtual ~cobraPlayEvent();

   void setHash(QString hash);
   QString hash() const;

   void setCommand(int x);
   int command() const;

   enum PlayCommands {
       Play, Reject
   };

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
   int              m_iCommand;
   QString          m_sHash;
};

#endif // PLAY_EVENT_H
