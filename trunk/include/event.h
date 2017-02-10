/*******************************************************************************
Copyright (c) 2012, All rights reserved.
	Terry Meacham <terry.t.meacham@gmail.com>
	Andrew Hicks <andrew.t.hicks@gmail.com>
	Nick Huskinson <nhuskinson@gmail.com>
	Jiake Liu  <jiakeliu@gmail.com>
*******************************************************************************/

#ifndef EVENT_H
#define EVENT_H

#include <QEvent>
#include <QString>
#include <QtGui>
#include <QtNetwork/QAbstractSocket>
#include <stdint.h>

#define cobraNetEventType               (QEvent::User+1)
#define cobraStateEventType             (cobraNetEventType+1)
#define cobraAuthEventType              (cobraNetEventType+2)
#define cobraChatEventType              (cobraNetEventType+3)
#define cobraTransferEventType          (cobraNetEventType+4)
#define cobraClipUpdateEventType        (cobraNetEventType+5)
#define cobraTimelineEventType          (cobraNetEventType+6)
#define cobraPlayEventType              (cobraNetEventType+7)
/* THIS value is the maximum allowable value... any new values must come
 * before it!
 */
#define cobraNetEventTypeMax            (cobraNetEventType+8)

inline bool
validEvent(int type) {
    return (type > cobraNetEventType && type < cobraNetEventTypeMax);
}

#define cobraConcurrentTransfers    (5)
#define cobraChunkSize              (4096)

/**
 * @typedef cobraId event.h "event.h"
 *
 * typedef for the id value used to track connections
 */
typedef unsigned int cobraId;

#define NO_ID           (~0x0U)
#define NO_SERVER       (~0x1U)
#define BROADCAST       (~0x2U)
#define SERVER          (~0xecc8ecc8U)
#define COBRA_ID_START  (0x10)

class cobraNetConnection;

#define CHAT_NOTIFY(x) "<font color=grey>" x "</font>"
#define CHAT_MESSAGE(x) "<font color=black>" x "</font>"
#define CHAT_AWAY(x) "<font color=red>" x "</font>"
#define CHAT_ONLINE(x) "<font color=green>" x "</font>"

 /**
  * @class cobraNetEvent event.h "event.h"
  *
  * The base class for handling events given particular parent cobraNetConnection.
  * If the data being recieved is a large amount of data, pull the necessary data,
  * and write the rest directly to disk (file transfers, primarily).
  */
class cobraNetEvent : public QEvent {
public:
    cobraNetEvent(int type);
    cobraNetEvent(cobraNetEvent& event);
    virtual ~cobraNetEvent();

    void setSource(cobraId id) { m_idSource = id; }
    cobraId source() const { return m_idSource; }
    void setDestination(cobraId id) { m_idDestination = id; }
    cobraId destination() const { return m_idDestination; }

    /**
     * @fn bool handled()
     * Indicate whether the event is being processed via QT Event system.  If so,
     * then the event will be freed automatically at the end of the current event cycle,
     * otherwise, it will be freed when its reference count reaches 0.
     * @param set Value to set the handled flag to.
     */
    bool handled() const;

    /**
     * @fn void setHandled(bool set)
     * Indicate whether the event is being processed via QT Event system.  If so,
     * then the event will be freed automatically at the end of the current event cycle.
     * Set to false to indicate that the event should be cleaned up when its reference count
     * reaches 0.
     * @param set Value to set the handled flag to.
     */
    void setHandled(bool set);

    /**
     * @fn int get(int cnt)
     * This is used to get an instance of an event before it has been sent
     * for handled. This really is only intended for calls which result in
     * the event getting sent to a thread through an InvokeMethod call.
     * @param cnt The number of elements to get.
     * @return The number of elements currently holding the object; -1 if handled.
     */
    int get(int cnt = 1);

    /**
     * @fn int put()
     * This is used to put an instance of an event after it has been handled.
     * When handled by the QTEvent system the Event will be cleaned up the
     * iteration after the current iteration in the main loop.  As a result,
     * this should really only be used on events which are not passed directly
     * into the QT Event system!
     * @return The number of elements currently holding the object; -1 if handled.
     */
    int put();

    /**
     * @fn bool isResponse() const
     * Used to determine if a given event is directed toward a client or server.
     * @return true if the event is a response.
     */
    bool isResponse() const {
        return m_bResponse;
    }

    /**
     * @fn void setResponse(bool is)
     * Set whether this even is intended
     * @return true if the event is a response.
     */
    void setResponse(bool isresponse) {
        m_bResponse = isresponse;
    }

    /**
     * @fn bool isRequest() const
     * Used to determine if a given event is directed toward a client or server.
     * @return true if the event is a request.
     */
    bool isRequest() const {
        return !m_bResponse;
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
     * Note: Sub-classes of cobraNetEvent only need to use the copy constructor
     * and then safely cleanup any internal data.
     * @return A pointer to the copied cobra event.
     */
    virtual cobraNetEvent* duplicate() = 0;

protected:
    /* Ref count is only used when m_bHandled is false! */
    QSemaphore  m_semRefcount;
    bool        m_bHandled;
    bool        m_bResponse;
    cobraId     m_idDestination;    /* Server ID */
    cobraId     m_idSource;         /* My ID */
};

/**
 * @class cobraNetEventHandler event.h "event.h"
 *
 * The cobraNetEventHandler is used to handle specific network events.
 * It may do this in the current thread or in its own thread, but
 * and interaction done with UI elements MUST be done in the MainThread
 * (QApplication::instance()->thread()).  If the handler sends a
 * received event to a handler thread, it should return that the event
 * was handle successfully, at which point, the recieving thread needs
 * to be notified (emit should work nicely) and the that thread must
 * then handle deletion of the memory.
 *
 * <b>NOTE: ONLY cobraNetEvent derivatives can be handled by cobraNetEventHandlers.
 *       If this rule is not obeyed, there may be issues with deletion of the events.</b>
 */
class cobraNetEventHandler {

public:
    cobraNetEventHandler(QString name, int event);
    cobraNetEventHandler(cobraNetEventHandler& event);
    virtual ~cobraNetEventHandler();

public:
    virtual bool handleEvent(cobraNetEvent* event) = 0;

    /**
     * @fn virtual cobraNetEvent* eventGenesis() const;
     * Generates an event of the type that it handles.
     */
    virtual cobraNetEvent* eventGenesis() const = 0;

    /**
     * @fn QString name()
     * Getter function for a human readable handler name.
     * @return QString name of the handler.
     */
    QString name() { return m_sName; }

    /**
     * @fn int type()
     * Getter function for the type of even this handler can handle.
     * @return QString name of the handler.
     */
    int     type() { return m_iType; }

    /**
     * @fn int put()
     * This may look funny because we're using the semaphore for reference counting.
     */
    int     put();

    /**
     * @fn int get()
     * This may look funny because we're using the semaphore for reference counting.
     */
    int     get();

protected:
    virtual bool handleServerEvent(cobraNetEvent* event) = 0;

protected:
    QString    m_sName;
    int        m_iType;
    QSemaphore m_semRef;
};

#include "state_event.h"
#include "chat_event.h"
#include "auth_event.h"
#include "clip_event.h"
#include "timeline_event.h"
#include "play_event.h"
#include "transfer.h"

#endif // EVENT_H
