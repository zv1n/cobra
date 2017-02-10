/*******************************************************************************
Copyright (c) 2012, All rights reserved.
	Terry Meacham <terry.t.meacham@gmail.com>
	Andrew Hicks <andrew.t.hicks@gmail.com>
	Nick Huskinson <nhuskinson@gmail.com>
	Jiake Liu  <jiakeliu@gmail.com>
*******************************************************************************/

#ifndef COBRALISTWIDGET_H
#define COBRALISTWIDGET_H

#include <QTreeWidget>
#include "clip.h"

class cobraTreeWidget : public QTreeWidget, public cobraClipList
{
    Q_OBJECT
public:
    cobraTreeWidget(QWidget *parent = 0);

    bool setName(QString db);

    bool updateClip(cobraClip& clip);
    bool removeClip(int uid);
    bool addClip(cobraClip& clip);

    void getSelectedUids(QVector<int>& uids);
    void getCheckedUids(QVector<int>& uids);
    void getCheckedUidsDownload(QVector<int>& uids);

    void synchronized();
    bool syncable() const;

    void showUpload(bool t);
    void showDownload(bool t);
    void showUid(bool t);

    void configure();
    void repopulateTree();

    enum cobraCondition {
        Update = 0x1, Add = 0x2
    };
    void setAutoCheck(int item, int condition);
    void setUpload(int uid, bool t);
    void setDownload(int uid, bool t);
    void uncheckAll(int id);

signals:
    void setSyncable(bool cansync);
    void downloadSelected(bool dl);

public slots:
    void clipItemChanged(QTreeWidgetItem* item, int index);
    void setFilter(QString);

public:
    enum ClipColumns{
        Upload, Download, ID, Hash, Title, Description, Tags, ModifiedTime
    };

protected:

    bool    m_bChecked;
};

#endif // COBRALISTWIDGET_H
