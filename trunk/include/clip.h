/*******************************************************************************
Copyright (c) 2012, All rights reserved.
	Terry Meacham <terry.t.meacham@gmail.com>
	Andrew Hicks <andrew.t.hicks@gmail.com>
	Nick Huskinson <nhuskinson@gmail.com>
	Jiake Liu  <jiakeliu@gmail.com>
*******************************************************************************/

#ifndef CLIP_H
#define CLIP_H

#include <QSqlDatabase>
#include <QtSql>

#include "clip.h"

/**
 * @class cobraClip clip.h "clip."
 *
 * The cobraClip handles all underlying sqllite and abstracts it.
 * This will allow functions to be called and just return file titles and
 * fields.  You can then poopulate the file lists with this information.
 */
class cobraClip {

public:
    cobraClip();
    virtual ~cobraClip();

    /**
     * @fn QString getTitle() 
     * @return the title of a Clip
     */
    QString getTitle() const {
        return (m_sTitle);
    }

    /**
     * @fn QString getHash() 
     * @return the hash of a Clip
     */
    QString getHash() const {
        return(m_sHash);
    }

    /**
     * @fn QString getPath() 
     * @return the path of a Clip
     */
    QString getPath() const {
        return(m_sPath);
    }

    /**
     * @fn QString getExtension()
     * @return the extension of a Clip
     */
    QString getExtension() const {
        return(m_sExtension);
    }

    /**
     * @fn QString getTags() 
     * @return the tag of a Clip
     */
    QString getTags()  const{
        return(m_sTags);
    }

    /**
     * @fn QString getModifiedTime() 
     * @return the modification time of a Clip
     */
    QString getModifiedTime() const {
        return(m_sModifiedTime);
    }

    /**
     * @fn QString getDescription()
     * @return the description of a Clip
     */
    QString getDescription() const {
        return(m_sDescription);
    }

    /**
     * @fn int getSize() 
     * @return the size of a Clip
     */
    int getSize() const {
        return(m_iSize);
    }

    /**
     * @fn int getUid()
     * @return the unique id of a Clip
     */
    int getUid() const {
        return(m_iUid);
    }

    /**
     * @fn QString setTitle() 
     * sets the title of a Clip
     */
    void setTitle(QString title) {
        m_sTitle = title;
    }

    /**
     * @fn void setHash() 
     * sets the hash of a Clip
     */
    void setHash(QString hash) {
        m_sHash = hash;
    }

    /**
     * @fn void setPath() 
     * sets the path of a Clip
     */
    void setPath(QString path) {
        m_sPath = path;
    }

    /**
     * @fn void setExtension()
     * sets the extension of a Clip
     */
    void setExtension(QString extension) {
        m_sExtension = extension;
    }

    /**
     * @fn void setTags() 
     * sets the tag of a Clip
     */
    void setTags(QString tags) {
        m_sTags = tags;
    }

    /**
     * @fn void setModifiedTime() 
     * sets the modification time of a Clip
     */
    void setModifiedTime(QString time) {
        m_sModifiedTime = time;
    }

    /**
     * @fn void setDescription()
     * sets the description of a Clip
     */
    void setDescription(QString desc) {
        m_sDescription = desc;
    }

    /**
     * @fn void setSize() 
     * sets the size of a Clip
     */
    void setSize(int size) {
        m_iSize = size;
    }

    /**
     * @fn void setUid()
     * sets the unique id of a Clip
     */
    void setUid(int Uid) {
        m_iUid = Uid;
    }

//public slots:

//signals:

protected:

    QString                             m_sTitle;
    QString                             m_sHash;
    QString                             m_sPath;
    QString                             m_sTags;
    QString                             m_sModifiedTime;
    QString                             m_sDescription;
    QString                             m_sExtension;
    int                                 m_iSize;
    int                                 m_iUid;
    
};

class cobraClipList {
public:
    cobraClipList(QString dbName = ":memory:");
    virtual ~cobraClipList();

   virtual bool                 setName(QString dbName);
   QString                      getName() const { return m_sDBName; }

   void                         enumClips(QVector<int>& vector);
   cobraClip                    getClip(int Uid);
   cobraClip                    getClipByHash(QString hash);
   virtual bool                 updateClip(cobraClip& clip);
   virtual bool                 removeClip(int Uid);
   virtual bool                 removeClipByHash(QString hash);
   virtual bool                 addClip(cobraClip& clip);

   virtual bool                 contains(int Uid);
   virtual bool                 containsHash(QString hash);
   unsigned long                totalSize() const;
   bool                         isValid() const;

protected:
   bool                         sqlQuery(QString&);
   static int                   nextList();

protected:
   QString                      m_sDBName;
   QSqlDatabase                 m_dbDatabase;

   static int                   m_iLists;
};

#endif // clip_H


