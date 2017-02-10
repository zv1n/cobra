/*******************************************************************************
Copyright (c) 2012, All rights reserved.
	Terry Meacham <terry.t.meacham@gmail.com>
	Andrew Hicks <andrew.t.hicks@gmail.com>
	Nick Huskinson <nhuskinson@gmail.com>
	Jiake Liu  <jiakeliu@gmail.com>
*******************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <clip.h>
#if !defined(NO_AUDIO)
#include <Phonon>
#endif

class cobraTransferController;
class cobraClipDialog;
class cobraClipList;
class Preferences;
class Transfersdlg;
class cobraTimelineDialog;
class markerDialog;

namespace Ui {
    class MainWindow;
}

class MainWindow;
typedef bool (MainWindow::*metaCmdEntry)(QString);

typedef QMap<QString, metaCmdEntry> metaCmdMap;
typedef metaCmdMap::Iterator mceIterator;
typedef metaCmdMap::ConstIterator mceConstIterator;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();


    /**
     * @fn virtual bool eventFilter(QObject* obj, QEvent* event)
     *
     * This function is used to remove enter key presses from the send box
     * and trigger a send event.
     */
    virtual bool eventFilter(QObject* obj, QEvent* event);

    bool focusFilter(QObject* obj, QEvent* event);

    bool updateLocalListDb(QString db);
    bool updateTimelineDb(QString db);
    bool updateServerDb(QString db);

    bool localLimitReached(unsigned long add) const;
    bool serverLimitReached(unsigned long add) const;
    unsigned long localSize() const;
    unsigned long serverSize() const;

    void playServerHash(const QString& hash);
    void playLocal(const cobraClip& clip);
    void playRemote(const cobraClip& clip);

    void notifyTransferComplete(QString hash);

public slots:
    void connected(bool cn);
    void newConnection(int id);

private slots:
    bool sendChat();
    bool sendAwayStatus();
    bool sendOnlineStatus();

    /* Meta Functions */
    void on_actionPreferences_triggered();
    void on_actionConnect_triggered();
    void on_actionFile_List_toggled(bool);
    void on_actionClip_Cue_toggled(bool);
    void on_actionServer_list_toggled(bool);
    void on_actionChat_Window_toggled(bool);
    void on_actionFile_Info_toggled(bool);
    void on_actionTransfers_triggered();
    void on_actionSelectUpload_triggered();
    void on_actionAddClip_triggered();
    void on_actionRemoveClip_triggered();
    void on_actionEditClip_triggered();
    void on_actionSync_triggered();
    void on_actionDownload_triggered();
    void on_actionAddTimeline_triggered();
    void on_actionMarker_Toolbar_triggered(bool);
    void on_actionAway_triggered();
    void on_actionOnline_triggered();
    void on_actionAddMark_triggered();
    void on_actionRemove_Timeline_triggered();
    void on_actionExportFileList_triggered();
    void on_actionExportTimelineList_triggered();
    void on_actionMute_triggered(bool checked);
    void on_actionPlay_triggered();

protected:
    bool setName(QString);
    bool metaConnect(QString);
    bool setAway(QString);

protected:
    //void setConnectState(bool connected);
    bool processChatCommand(QString cmd);

protected:
    bool registerMetaCommand(metaCmdEntry func, QString cmd);
    bool processMetaCommand(QString cmd);

    void sendLocalUpdates();
    void refreshServerList();

protected:
    metaCmdMap m_msfMetaMap;
    Ui::MainWindow *ui;
    QString m_cUsername;
    Preferences* m_pDialog;
    Transfersdlg* m_dTransfers;
    cobraClipList* m_cclFocused;
    cobraClipDialog* m_ccdDialog;
    cobraTimelineDialog* m_tlDialog;
    markerDialog* m_mDialog;
    cobraTransferController* m_ctcTransfer;

#if !defined(NO_AUDIO)
    Phonon::AudioOutput* m_aoAudioOutput;
    Phonon::MediaObject* m_moAudioFile;
#endif
};


#endif // MAINWINDOW_H
