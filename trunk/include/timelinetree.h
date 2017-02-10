/*******************************************************************************
Copyright (c) 2012, All rights reserved.
	Terry Meacham <terry.t.meacham@gmail.com>
	Andrew Hicks <andrew.t.hicks@gmail.com>
	Nick Huskinson <nhuskinson@gmail.com>
	Jiake Liu  <jiakeliu@gmail.com>
*******************************************************************************/

#ifndef TIMELINETREE_H
#define TIMELINETREE_H

#include <QTreeWidget>
#include "timeline.h"

class cobraTimelineTree : public QTreeWidget, public cobraTimelineList
{
    Q_OBJECT
public:
    enum {Title, Time, Description, TID, MID };
    cobraTimelineTree(QWidget *parent = 0);
    bool setName(QString db);
    void configure();
    bool updateTimeline(cobraTimeline& tl);
    bool removeTimeline(int uid);
    bool addTimeline(cobraTimeline& tl);
    bool addMark(cobraMark& mark);
    bool removeMark(int Uid);
    bool updateMark(cobraMark &mark);
    void getSelectedUids(QVector<int>& uids);
};

#endif // TIMELINETREE_H
