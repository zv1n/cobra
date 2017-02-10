/*******************************************************************************
Copyright (c) 2012, All rights reserved.
	Terry Meacham <terry.t.meacham@gmail.com>
	Andrew Hicks <andrew.t.hicks@gmail.com>
	Nick Huskinson <nhuskinson@gmail.com>
	Jiake Liu  <jiakeliu@gmail.com>
*******************************************************************************/

#ifndef CLIP_EVENT_H
#define CLIP_EVENT_H

#include "event.h"
#include "clip.h"

class cobraClipList;
class cobraClipUpdateEvent;

/**
 * @class cobraClipUpdateEventHandler event.h "event.h"
 *
 * The cobraClipUpdateEventHandler is used to handle incoming authorization requests.
 */
class cobraClipUpdateEventHandler : public cobraNetEventHandler {

public:
    cobraClipUpdateEventHandler();
    cobraClipUpdateEventHandler(cobraClipUpdateEventHandler& event);
    virtual ~cobraClipUpdateEventHandler();

    void setServerList(cobraClipList* server) {
        m_cclServer = server;
    }

    cobraClipList* serverList() const {
        return m_cclServer;
    }

    void setLocalList(cobraClipList* local) {
        m_cclLocal = local;
    }

    cobraClipList* localList() const {
        return m_cclLocal;
    }

public:
    virtual bool handleEvent(cobraNetEvent* event);

    /**
     * @fn virtual cobraNetEvent* eventGenesis() const;
     * Generates an event of the type that it handles.
     */
    virtual cobraNetEvent* eventGenesis() const;

protected:
    virtual bool handleServerEvent(cobraNetEvent* event);
    bool handleAdd(cobraClipUpdateEvent* event, cobraClipList*);
    bool handleUpdate(cobraClipUpdateEvent* event, cobraClipList*);
    bool handleSyncRequest(cobraClipUpdateEvent* event, cobraClipList*);
    bool handleBlindUpdate(cobraClipUpdateEvent* event, cobraClipList*);
    bool handleFileRequest(cobraClipUpdateEvent* event, cobraClipList*);

protected:
    cobraClipList*    m_cclServer;
    cobraClipList*    m_cclLocal;
};

/**
 * @class cobraClipUpdateEvent event.h "event.h"
 *
 * The event used to communicate authentication information to the server and other
 * machines.
 */

class cobraClipUpdateEvent : public cobraNetEvent {
public:
   cobraClipUpdateEvent();
   cobraClipUpdateEvent(cobraClipUpdateEvent& state);
   virtual ~cobraClipUpdateEvent();

   cobraClip clip() const;
   void setClip(const cobraClip& clip);

   void setCommand(int x);
   int command() const;

   enum ClipCommands {
       Update, Add, Remove, RequestSync, BlindUpdate, FileRequest, FileResponse, Reject
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
   int          m_iCommand;
   cobraClip    m_ccClip;
};


#endif // CLIP_EVENT_H
