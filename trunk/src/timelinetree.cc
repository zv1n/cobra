/*******************************************************************************
Copyright (c) 2012, All rights reserved.
	Terry Meacham <terry.t.meacham@gmail.com>
	Andrew Hicks <andrew.t.hicks@gmail.com>
	Nick Huskinson <nhuskinson@gmail.com>
	Jiake Liu  <jiakeliu@gmail.com>
*******************************************************************************/

#include <QtGui>
#include "timelinetree.h"
#include <QStandardItem>
#include "debug.h"

cobraTimelineTree::cobraTimelineTree(QWidget *parent) :
    QTreeWidget(parent), cobraTimelineList()
{

}

void
cobraTimelineTree::configure()
{

    setColumnWidth(Title, 100);
    setColumnWidth(Time, 80);
    setColumnWidth(Description, 300);
    setColumnHidden(TID, true);
    setColumnHidden(MID, true);
}

bool
cobraTimelineTree::setName(QString name)
{
    clear();
    if (!cobraTimelineList::setName(name))
        return false;

    QVector<int> timelines;
    enumTimelines(timelines);

    for (int x=0; x<timelines.size(); x++) {
        QTreeWidgetItem* item = new QTreeWidgetItem();
        item->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
        cobraTimeline timeline = getTimeline(timelines[x]);
        if (timeline.getUid() != timelines[x]) {
            debug(ERROR(CRITICAL), "Failed to get timeline item: %d\n", timelines[x]);
            delete item;
            continue;
        }

        item->setText(Title, timeline.getTitle());
        item->setText(Description, timeline.getDescription());
        item->setText(TID, QString::number(timeline.getUid()));
        item->setText(Time, timeline.getStartTime().toString("h:m:s.z"));

        this->addTopLevelItem(item);

        QVector<int> marks;
        enumTimelineMarkers(marks, timeline);

        for (int y=0; y<marks.size(); y++) {
            QTreeWidgetItem* markitem = new QTreeWidgetItem();

            cobraMark mark = getMark(marks[y]);
            if (mark.getUid() != marks[y]) {
                debug(ERROR(CRITICAL), "Failed to get mark item: %d\n", marks[y]);
                delete markitem;
                continue;
            }

            markitem->setText(Description, mark.getComment());
            markitem->setText(Time, "+" + QString::number(mark.getTimeOffset()));
            markitem->setText(MID, QString::number(mark.getUid()));

            item->addChild(markitem);
        }

    }

    return true;
}

bool
cobraTimelineTree::updateTimeline(cobraTimeline& tl)
{
    bool ret = cobraTimelineList::updateTimeline(tl);
    if (!ret) {
        debug(CRITICAL, "TimelineList error");
                return ret;
    }

    QString tlid = QString::number(tl.getUid());
    QString tltitle = tl.getTitle();
    QString tldesc = tl.getDescription();

    QList<QTreeWidgetItem*> c = this->findItems(tlid, Qt::MatchExactly, TID);

    if(c.size()!=1)
    {
        debug(ERROR(HIGH), "Failed to find specified ID!\n");
        return false;
    }

    QTreeWidgetItem* itm = c.takeFirst();

    itm->setText(Title, tltitle);
    itm->setText(Description, tldesc);
    itm->setText(TID, tlid);

    //SCHEMA: TITLE | COMMENT | UID (HIDDEN) | STARTTIME (HIDDEN)

    return true;
}

bool
cobraTimelineTree::removeTimeline(int uid)
{
    bool ret = cobraTimelineList::removeTimeline(uid);
    if(!ret)
        return ret;

    QList<QTreeWidgetItem*> c = this->findItems(QString::number(uid), Qt::MatchExactly, TID);

    if(c.size()!=1)
    {
        debug(ERROR(HIGH), "Failed to find specified ID!\n");
        return false;
    }

    delete c.takeFirst();

    return true;
}

bool
cobraTimelineTree::addTimeline(cobraTimeline& tl)
{
    bool ret = cobraTimelineList::addTimeline(tl);
    if (!ret)
        return ret;

    QTreeWidgetItem *itm = new QTreeWidgetItem(this);
    itm->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);

    QString tlid = QString::number(tl.getUid());
    QString tltitle = tl.getTitle();
    QString tlstarttime = tl.getStartTime().toString("h:m:s.z");
    QString tldesc = tl.getDescription();

    itm->setText(Title, tltitle);
    itm->setText(Description, tldesc);
    itm->setText(TID, tlid);
    itm->setText(Time, tlstarttime);

     //SCHEMA: TITLE | COMMENT | <BLANK> (MARK TIMEOFFSET) | UID (HIDDEN) | STARTTIME (HIDDEN)

    this->addTopLevelItem(itm);

    return true;
}

bool
cobraTimelineTree::addMark(cobraMark &mark)
{
    bool ret = cobraTimelineList::addMark(mark);
    if(!ret)
        return ret;

    cobraTimeline tline = getTimeline(mark.getTimelineKey());

    QList<QTreeWidgetItem*> c = this->findItems(QString::number(tline.getUid()), Qt::MatchExactly, TID);

    if (c.size() != 1) {
        QMessageBox::critical(this, "Timeline Failure!", "No timeline with specified ID exists!");
        return false;
    }

    QTreeWidgetItem* paritm = c[0];
    QTreeWidgetItem* chitm = new QTreeWidgetItem(paritm); //child

    QString muid = QString::number(mark.getUid());
    QString mtimeoffset = "+" + QString::number(mark.getTimeOffset());
    QString mcomment = mark.getComment();

    chitm->setText(Description, mcomment);
    chitm->setText(Time, mtimeoffset);
    chitm->setText(MID, muid);

     //SCHEMA: (HIDDEN) | COMMENT | TIMEOFFSET | UID (HIDDEN)

    paritm->addChild(chitm);

    return true;
}

bool
cobraTimelineTree::removeMark(int uid)
{
    bool ret = cobraTimelineList::removeMark(uid);
    if(!ret)
        return ret;

    QList<QTreeWidgetItem*> c = this->findItems(QString::number(uid), Qt::MatchExactly, MID);

    if(c.size()!=1)
    {
        debug(ERROR(HIGH), "Failed to find specified ID!\n");
        return false;
    }

    delete c.takeFirst();

    return true;
}

bool
cobraTimelineTree::updateMark(cobraMark &mark)
{
    bool ret = cobraTimelineList::updateMark(mark);
    if(!ret)
        return ret;

    QString muid = QString::number(mark.getUid());
    QString mtimeoffset = "+" + QString::number(mark.getTimeOffset());
    QString mcomment = mark.getComment();


    QList<QTreeWidgetItem*> c = this->findItems(muid, Qt::MatchExactly, MID);

    if(c.size()!=1)
    {
        debug(ERROR(HIGH), "Failed to find specified ID!\n");
        return false;
    }

    QTreeWidgetItem* itm = c.takeFirst();

    itm->setText(Description, mcomment);
    itm->setText(Time, mtimeoffset);
    itm->setText(MID, muid);

    //SCHEMA: (HIDDEN) | COMMENT | UID (HIDDEN) | TIMEOFFSET (HIDDEN)

    return true;
}

void
cobraTimelineTree::getSelectedUids(QVector<int>& uids)
{
    QList<QTreeWidgetItem *> items = this->selectedItems();
    for(int i = 0; i < items.size(); i++) {
        QTreeWidgetItem *itm = items.at(i);

        if (!itm)
            continue;

        uids.append(itm->text(MID).toInt(0,10));
    }
}
