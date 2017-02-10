/*******************************************************************************
Copyright (c) 2012, All rights reserved.
	Terry Meacham <terry.t.meacham@gmail.com>
	Andrew Hicks <andrew.t.hicks@gmail.com>
	Nick Huskinson <nhuskinson@gmail.com>
	Jiake Liu  <jiakeliu@gmail.com>
*******************************************************************************/

#ifndef STATE_EVENT_H
#define STATE_EVENT_H

#include "event.h"

class cobraStateEvent;
class cobraStateEventHandler;

/**
 * @class cobraStateEvent event.h "event.h"
 *
 * The base class for handling events given a
 * particular parent cobraNetConnection.
 */

class cobraStateEvent : public cobraNetEvent {
public:
   cobraStateEvent();
   cobraStateEvent(cobraStateEvent& state);
   virtual ~cobraStateEvent();

public:

   /**
    * @fn void setState(int state)
    * Sets the state of the event.
    */
   void setState(int state) {
       m_iState = state;
   }

   /**
    * @fn int getState() const
    * Gets the current state.
    */
   int getState() const {
       return m_iState;
   }

   /**
    * @fn void setFlag(int flag)
    * Used to set a state flag.
    * @param flag The flag to be set.
    */
   void setFlag(int flag) {
       m_iFlags |= flag;
   }

   /**
    * @fn void clearFlag(int flag)
    * Used to clear a state flag.
    * @param flag The flag to be cleared.
    */
   void clearFlag(int flag) {
       m_iFlags &= ~flag;
   }

   /**
    * @fn void toggleFlag(int flag)
    * Used to toggle a state flag.
    * @param flag The flag to be toggle.
    */
   void toggleFlag(int flag) {
       m_iFlags ^= flag;
   }

   /**
    * @fn virtual int serialize(QDataStream& stream)
    * Serialize the data which is pending on the given cobraNetConnection.
    */
   virtual int serialize(QDataStream& stream);

   /**
    * @fn virtual int deserialize(QDataStream& stream)
    * Deserialize the data which is pending on the given cobraNetConnection.
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

   enum SocketState {
       ConnectingState,
       ConnectedState,
       ClosingState,
       ConnectionRefused,
       DisconnectedState
   };

   enum StateFlags {
       Forced = 0x1,
       AuthenticationFailure = 0x2
   };

protected:
   int  m_iFlags;
   int  m_iState;
};


/**
 * @class cobraStateEventHandler event.h "event.h"
 *
 * The cobraStateEventHandler is used to handle incoming state events.
 * Because this event handler interfaces with the User Interface, it
 * absolutly CAN NOT use threads to process incoming information.
 */
class cobraStateEventHandler : public cobraNetEventHandler {

public:
    cobraStateEventHandler();
    cobraStateEventHandler(cobraStateEventHandler& event);
    virtual ~cobraStateEventHandler();

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

#endif // STATE_EVENT_H
