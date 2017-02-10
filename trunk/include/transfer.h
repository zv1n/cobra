/*******************************************************************************
Copyright (c) 2012, All rights reserved.
	Terry Meacham <terry.t.meacham@gmail.com>
	Andrew Hicks <andrew.t.hicks@gmail.com>
	Nick Huskinson <nhuskinson@gmail.com>
	Jiake Liu  <jiakeliu@gmail.com>
*******************************************************************************/

#ifndef TRANSFER_H
#define TRANSFER_H

#include "event.h"

class cobraTransferFile;

/**
 * @class cobraTransferEvent event.h "event.h"
 *
 * The event used to communicate authentication information to the server and other
 * machines.
 */

class cobraTransferEvent : public cobraNetEvent {
public:
   cobraTransferEvent();
   cobraTransferEvent(cobraTransferEvent& state);
   virtual ~cobraTransferEvent();

   void setCommand(int cmd);
   int command() const;

   void setOffset(qint64 offset);
   qint64 offset() const;

   void setSize(qint64 size);
   qint64 size() const;

   /* This should contain the HASH of the file to be sent. */
   void setHash(QByteArray& hash);
   const QByteArray& hash() const;

   void setData(QByteArray& data);
   const QByteArray& data() const;
   void clearData();

   bool fromFile(cobraTransferFile* file);

   QString extension();
   void setExtension(QString path);

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

   enum TransferCommands {
       Request,
       Accept,
       Reject,
       Exists,
       Chunk,
       Complete,
       Resend
   };

protected:
    int             m_iCommand;
    qint64          m_iOffset;
    qint64          m_iSize;
    QByteArray      m_baHash;
    QByteArray      m_baData;
    QString         m_sExtension;
};

class cobraNetEventThread;
class cobraTransferController;

#define CURRENT_OFFSET (-1)

/**
 * @class cobraTransferFile event.h "event.h"
 *
 * The cobraTransferFile class is intended to provide necessary functions to make generating
 * file transfer events easier.  The goal is to use this object to store the path to the file
 * prior to activating the transfer event.
 * Outbound:
 * Once a file transfer is started, this class will handle reading from the file and sending
 * the data via an event to its destination.
 * Inbound:
 * Once a file transfer is started, this class will handle writing incoming events to the file.
 *
 * Each file transfer will have its own unique ID generated by the sending element.
 */
class cobraTransferFile : public QFile {
    Q_OBJECT

public:

    cobraTransferFile(QString);
    ~cobraTransferFile();

    bool isActive() const;
    void activate(bool enable = true);

    bool isSending() const;
    void setSending(bool sending = true);

    bool isPendingCompletion() const;

    bool is(const QByteArray& hash) const;

    void setExpectedHash(const QByteArray& hash);
    QByteArray expectedHash() const;
    QByteArray hash();
    QByteArray currentHash();

    static QByteArray hashFile(QString path);

    void setDestination(cobraId dest);
    void setSource(cobraId src);

    void setPath(QString path);

    cobraId destination() const;
    cobraId source() const;

    bool sendChunk(cobraNetEventThread* thread, qint64 chunk = 1024);
    int recieveChunk(cobraTransferEvent* event);

    enum ChunkReceipt {
        TransferError = 0,
        TransferComplete = 1,
        TransferIncomplete = 2,
        TransferIncompleteNoncontiguous = 3
    };

    int isComplete() const;

    bool transferComplete();
    bool resendChunk(qint64 chunk, qint64 offset);

protected:
    void setPendingCompletion(bool pending = true);

protected:
    QByteArray  m_baHash;
    QByteArray  m_baExpectedHash;

    cobraId     m_idSource;
    cobraId     m_idDestination;

    bool        m_bActive;
    bool        m_bPendingCompletion;
    bool        m_bSending;
};

/**
 * @class cobraTransferController event.h "event.h"
 *
 * The class is used by the file transfer API to keep track of file transfer operations.
 * Internal this object is all pending outbound file transfers.  File transfers are
 * allowed to schedule in queue, which only allows a certain number of files open in
 * the queue at a time.  Once a file transfer has completed, the next file in the
 * pending list is opened, and the transfer is started.  The inbound tansfer mechanism is
 * handled as well, but is controlled via the cobraTransferFileController object in the
 * cobraTransferEventHandler class.
 *
 */
class cobraTransferController : public QObject {
    Q_OBJECT

public:
   cobraTransferController(int concurrent = cobraConcurrentTransfers, QObject* parent = NULL);
   virtual ~cobraTransferController();

public slots:
   bool transferTrigger();

public:

   void setTransferCount(int con);
   int transferCount() const;

   void setChunkSize(qint64 chunk);
   qint64 chunkSize() const;

   void setInterval(int interval);
   int interval() const;

   int recieveChunk(cobraTransferEvent* event);

   bool interceptEvent(cobraTransferEvent* event);
   bool addTransfer(cobraTransferFile* file);
   bool removeTransfer(cobraTransferFile* file);

   bool initialize(cobraNetEventThread* parent = NULL);
   void cleanup();

protected:
   cobraTransferFile* getFile(const QByteArray& hash);

protected:
   int                          m_iNextTransfer;
   int                          m_iConcurrentTransfers;
   qint64                       m_iChunkSize;

   cobraNetEventThread*         m_netParent;
   QTimer                       m_tTransferTimer;
   QVector<cobraTransferFile*>  m_vcftTransfers;

public:
   static bool addPendingTransfer(cobraTransferFile* file);
   static cobraTransferFile* getPendingTransfer(const QByteArray& hash);
   static QMap<QByteArray, cobraTransferFile*>  m_vcftPending;
};

class MainWindow;

/**
 * @class cobraTransferEventHandler event.h "event.h"
 *
 * The cobraTransferEventHandler is used to handle incoming authorization requests.
 */
class cobraTransferEventHandler : public cobraNetEventHandler, public cobraTransferController {

public:
    cobraTransferEventHandler();
    cobraTransferEventHandler(cobraTransferEventHandler& event);
    virtual ~cobraTransferEventHandler();

public:
    virtual bool handleEvent(cobraNetEvent* event);

    /**
     * @fn virtual cobraNetEvent* eventGenesis() const;
     * Generates an event of the type that it handles.
     */
    virtual cobraNetEvent* eventGenesis() const;

    void setWindow(MainWindow* main);

protected:
    virtual bool handleServerEvent(cobraNetEvent* event);

protected:
    MainWindow* m_mwMain;
};

class cobraTransferStatistics {
public:
    cobraTransferStatistics();
    ~cobraTransferStatistics();

    struct metrics {
        QString path;
        unsigned long size;
        unsigned long current;
        unsigned long time;
    };

    typedef QMap<QByteArray, metrics> cobraStatMap;

    static cobraTransferStatistics* instance();

    void updatePath(const QByteArray&, QString path);
    void updateSize(const QByteArray&, unsigned long size);
    void updateCurrent(const QByteArray&, unsigned long current);
    void updateTime(const QByteArray&, unsigned long time);
    void getStatistics(cobraStatMap& map) const;

protected:
    mutable QReadWriteLock  m_rwlStatLock;
    cobraStatMap            m_mumMetrics;
};


#endif // TRANSFER_H
