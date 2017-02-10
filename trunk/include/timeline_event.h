/*******************************************************************************
Copyright (c) 2012, All rights reserved.
	Terry Meacham <terry.t.meacham@gmail.com>
	Andrew Hicks <andrew.t.hicks@gmail.com>
	Nick Huskinson <nhuskinson@gmail.com>
	Jiake Liu  <jiakeliu@gmail.com>
*******************************************************************************/

#ifndef TIMELINE_EVENT_H
#define TIMELINE_EVENT_H

#include "event.h"
#include "timeline.h"

class cobraTimelineList;
class cobraTimelineEvent;

/**
 * @class cobraTimelineEventHandler event.h "event.h"
 *
 * The cobraTimelineEventHandler is used to handle incoming authorization requests.
 */
class cobraTimelineEventHandler : public cobraNetEventHandler {

public:
    cobraTimelineEventHandler();
    cobraTimelineEventHandler(cobraTimelineEventHandler& event);
    virtual ~cobraTimelineEventHandler();

    void setTimelineList(cobraTimelineList* list);

public:
    virtual bool handleEvent(cobraNetEvent* event);

    /**
     * @fn virtual cobraNetEvent* eventGenesis() const;
     * Generates an event of the type that it handles.
     */
    virtual cobraNetEvent* eventGenesis() const;

    void setLastTimeline(QDateTime dt);

protected:
    virtual bool handleServerEvent(cobraNetEvent* event);
    bool handleGenerateTimeline(cobraTimelineEvent* event);
    bool handleGenerateMark(cobraTimelineEvent* event);

    void clientMark(cobraTimelineEvent* tup);
    void clientTimeline(cobraTimelineEvent* tup);
    void serverMark(cobraTimelineEvent* tup);
    void serverTimeline(cobraTimelineEvent* tup);

protected:
    QDateTime             m_dtLastTimeline;
    cobraTimelineList*    m_ctlTimeline;
};


/**
 * @class cobraTimelineEvent event.h "event.h"
 *
 * The event used to communicate authentication information to the server and other
 * machines.
 */

class cobraTimelineEvent: public cobraNetEvent {
public:
   cobraTimelineEvent();
   cobraTimelineEvent(cobraTimelineEvent& state);
   virtual ~cobraTimelineEvent();

   cobraTimeline timeline() const;
   void setTimeline(const cobraTimeline& timeline);

   cobraMark mark() const;
   void setMark(const cobraMark& mark);

   void setCommand(int x);
   int command() const;

   enum TimelineCommands {
       GenerateTimeline, GenerateMark, Reject
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
   cobraTimeline    m_ctTimeline;
   cobraMark        m_cmMark;
};

#endif // TIMELINE_EVENT_H
