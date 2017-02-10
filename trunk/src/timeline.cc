/*******************************************************************************
Copyright (c) 2012, All rights reserved.
	Terry Meacham <terry.t.meacham@gmail.com>
	Andrew Hicks <andrew.t.hicks@gmail.com>
	Nick Huskinson <nhuskinson@gmail.com>
	Jiake Liu  <jiakeliu@gmail.com>
*******************************************************************************/

#include "timeline.h"
#include "debug.h"

#include <QSqlQuery>
#include <QSqlTableModel>
#include <QSqlError>
#include <QSqlRecord>
#include <QMessageBox>
#include <QTableView>

cobraTimeline::cobraTimeline()
    : m_iUid(0)
{
    debug(LOW, "cobraTimeline initializing...\n");
}

cobraTimeline::~cobraTimeline()
{
    debug(LOW, "cobraTimeline dieing...\n");
}

cobraMark::cobraMark(cobraTimeline& timeline)
    : m_iTimeOffset(-1), m_iUid(0)
{
    m_iTimelineKey = timeline.getUid();
    debug(LOW, "cobraMark initializing...\n");
}

cobraMark::cobraMark()
    : m_iTimeOffset(-1), m_iUid(0)
{
    debug(LOW, "cobraMark initializing for copy...\n");
}

cobraMark::~cobraMark()
{
    debug(LOW, "cobraMark dieing...\n");
}

int cobraTimelineList::m_iLists;

int
cobraTimelineList::nextList() {
   return m_iLists++;
}

cobraTimelineList::cobraTimelineList(QString dbName)
{
    debug(LOW, "cobraTimelineList initializing using 1 argument, the dbName...\n");
    setName(dbName);
}

bool
cobraTimelineList::setName(QString dbName)
{
    m_sDBName = dbName;
    m_dbDatabase = QSqlDatabase::addDatabase("QSQLITE", QString("TimelineList%1").arg(nextList()));
    m_dbDatabase.setDatabaseName(m_sDBName);

    if (!m_dbDatabase.open()) {
        debug(ERROR(CRITICAL), "Failed to open DATABASE!\n");
        return false;
    }

    QSqlQuery query(m_dbDatabase);

    // table creation strings for timelines
    QString select = "SELECT * FROM cobraTimelines;";
    QString create = "CREATE TABLE  cobraTimelines "
            "(uid INTEGER PRIMARY KEY, title varchar(60),"
            "description varchar(140), timestarted datetime);";

    //creates table for new db
    if (!sqlQuery(select)) {
        debug(HIGH, "Uninitialized Database! Generating new one...\n");
        if (!sqlQuery(create)) {
            debug(ERROR(CRITICAL), "Failed to create requested database!\n");
            return false;
        }
    }

    // table creation strings for markers
    select = "SELECT * FROM cobraMarkers;";
    create = "CREATE TABLE  cobraMarkers "
            "(uid INTEGER PRIMARY KEY, timelineKey INTEGER,"
            "comment varchar(140), offset INTEGER);";

    //creates table for new db
    if (!sqlQuery(select)) {
        debug(HIGH, "Uninitialized Database! Generating new one...\n");
        if (!sqlQuery(create)) {
            debug(ERROR(CRITICAL), "Failed to create requested database!\n");
            return false;
        }
    }

    return true;
}

cobraTimelineList::~cobraTimelineList()
{
    m_dbDatabase.close();
    debug(LOW, "cobraTimelineList dieing...\n");
}


bool
cobraTimelineList::contains(int uid)
{
    //if (!m_dbDatabase.isOpen())
    //    return false;

    QSqlQuery query(m_dbDatabase);

    if (!query.exec("SELECT uid FROM cobraTimelines WHERE uid=" % QString::number(uid) % ";")) {
        debug(LOW, "Failed to exec..\n");
        return false;
    }

    if (!query.next()) {
        debug(LOW, "Failed to exec..\n");
        return false;
    }

    return (query.value(0).toInt() == uid);
}

bool
cobraTimelineList::isValid() const
{
    return true; //m_dbDatabase.isOpen();
}

cobraTimeline cobraTimelineList::getTimeline(int uid) const
{
    cobraTimeline ctTimeline;
    QSqlQuery query(m_dbDatabase);

    bool result = query.exec("SELECT uid,title,description,timestarted "
                             "FROM cobraTimelines WHERE uid=" % QString::number(uid) % ";");

    if (!result || !query.next()) {
        debug(ERROR(CRITICAL), "Failed to get specified timeline.\n");
        return ctTimeline;
    }

    ctTimeline.setUid(query.value(0).toInt());
    ctTimeline.setTitle(query.value(1).toString());
    ctTimeline.setDescription(query.value(2).toString());
    ctTimeline.setStartTime(query.value(3));

    return ctTimeline;
}

bool
cobraTimelineList::updateTimeline(cobraTimeline& timeline)
{
    QString updateString = "UPDATE cobraTimelines SET "
            "title='" % timeline.getTitle() % "',"
            "description='" % timeline.getDescription() %  "' "
            "WHERE uid=" % QString::number(timeline.getUid()) % ";";

    debug(LOW, "Updated: %s\n", qPrintable(updateString));
    return sqlQuery(updateString);
}

bool
cobraTimelineList::removeTimeline(int uid)
{
    QString deleteString = "DELETE FROM cobraTimelines WHERE uid=" % QString::number(uid) % ";";
    return sqlQuery(deleteString);
}

bool
cobraTimelineList::addTimeline(cobraTimeline& timeline)
{
    QSqlQuery query(m_dbDatabase);
    QString insertString =
            "INSERT INTO cobraTimelines (uid, title, description, timestarted) "
            "VALUES (NULL, "
            "'" % timeline.getTitle() % "', "
            "'" % timeline.getDescription() % "', "
            "'" % timeline.getStartTime().toString("yyyy-MM-dd HH:mm:ss") % "');";

    debug(LOW, "Inserted: %s\n", qPrintable(insertString));
    if (!query.exec(insertString)) {
        QSqlError err = m_dbDatabase.lastError();
        debug(ERROR(CRITICAL), "Failed to Query Database (%s).\n", qPrintable(m_dbDatabase.connectionName()));
        if (!err.databaseText().isEmpty())
            debug(ERROR(CRITICAL),"%s\n", qPrintable(err.databaseText()));
        if (!err.driverText().isEmpty())
            debug(ERROR(CRITICAL),"%s\n", qPrintable(err.driverText()));
        return false;
    }

    bool ok = false;
    timeline.setUid(query.lastInsertId().toInt(&ok));
    return ok;
}

cobraMark cobraTimelineList::getMark(int uid) const
{
    cobraMark cmMark;
    QSqlQuery query(m_dbDatabase);

    bool result = query.exec("SELECT uid, timelineKey, comment, offset "
                             "FROM cobraMarkers WHERE uid=" % QString::number(uid) % ";");

    if (!result || !query.next()) {
        debug(ERROR(CRITICAL), "Failed to get specified mark.\n");
        return cmMark;
    }

    cmMark.setUid(query.value(0).toInt());
    cmMark.setTimelineKey(query.value(1).toInt());
    cmMark.setComment(query.value(2).toString());
    cmMark.setTimeOffset(query.value(3).toInt());

    return cmMark;
}

bool
cobraTimelineList::updateMark(cobraMark& mark)
{
    QString updateString = "UPDATE cobraMarkers SET "
            "comment='" % mark.getComment() %  "' "
            "WHERE uid=" % QString::number(mark.getUid()) % ";";

    debug(LOW, "Updated: %s\n", qPrintable(updateString));
    return sqlQuery(updateString);
}

bool
cobraTimelineList::removeMark(int uid)
{
    QString deleteString = "DELETE FROM cobraMarkers WHERE uid=" % QString::number(uid) % ";";
    return sqlQuery(deleteString);
}

bool
cobraTimelineList::addMark(cobraMark& mark)
{
    QSqlQuery query(m_dbDatabase);
    QString insertString =
            "INSERT INTO cobraMarkers (uid, timelineKey, comment, offset) "
            "VALUES (NULL, "
            "'" % QString::number(mark.getTimelineKey()) % "', "
            "'" % mark.getComment() % "', "
            "'" % QString::number(mark.getTimeOffset()) % "');";

    debug(LOW, "Inserted: %s\n", qPrintable(insertString));
    if (!query.exec(insertString)) {
        QSqlError err = m_dbDatabase.lastError();
        debug(ERROR(CRITICAL), "Failed to Query Database (%s).\n", qPrintable(m_dbDatabase.connectionName()));
        if (!err.databaseText().isEmpty())
            debug(ERROR(CRITICAL),"%s\n", qPrintable(err.databaseText()));
        if (!err.driverText().isEmpty())
            debug(ERROR(CRITICAL),"%s\n", qPrintable(err.driverText()));
        return false;
    }

    bool ok = false;
    mark.setUid(query.lastInsertId().toInt(&ok));
    return ok;
}

bool
cobraTimelineList::sqlQuery(QString& string)
{
    QSqlQuery query(m_dbDatabase);

    if (!query.exec(string)) {
        QSqlError err = m_dbDatabase.lastError();
        debug(ERROR(CRITICAL), "Failed to Query Database.\n");
        if (!err.databaseText().isEmpty())
            debug(ERROR(CRITICAL),"%s\n", qPrintable(err.databaseText()));
        if (!err.driverText().isEmpty())
            debug(ERROR(CRITICAL),"%s\n", qPrintable(err.driverText()));
        return false;
    }

    debug(LOW,"This is the string that was submitted for the SqlQuery: %s\n", qPrintable(string));
    return true;
}

cobraTimeline
cobraTimelineList::getCurrentTimeline() const
{
    QString qstring = "SELECT MAX(uid) FROM cobraTimelines;";
    QSqlQuery query(m_dbDatabase);

    if (!query.exec(qstring)) {
        debug(ERROR(CRITICAL), "Failed to Query Database.\n");
        return cobraTimeline();
    }

    if (!query.next()) {
        debug(ERROR(CRITICAL), "Failed to Query Database.\n");
        return cobraTimeline();
    }

    return getTimeline(query.value(0).toInt());
}

void
cobraTimelineList::enumTimelines(QVector<int>& vector)
{
    QSqlQuery query("SELECT uid FROM cobraTimelines", m_dbDatabase);

    query.exec();

    while (query.next())
        vector.append(query.value(0).toInt());

    return;
}

void
cobraTimelineList::enumTimelineMarkers(QVector<int>& vector, cobraTimeline& timeline)
{
    QString enumString = "SELECT uid FROM cobraMarkers "
            "WHERE timelineKey=" % QString::number(timeline.getUid()) % ";";

    QSqlQuery query(enumString, m_dbDatabase);

    query.exec();

    while (query.next())
        vector.append(query.value(0).toInt());

    return;
}

