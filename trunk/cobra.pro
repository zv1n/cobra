######################################################################
# Automatically generated by qmake (2.01a) Wed Jan 11 21:01:37 2012
######################################################################
CONFIG += qtestlib

QT += network xml sql 

message("Building against QT Version: " $$QT_VERSION)

lessThan(QT_MAJOR_VERSION,4) | lessThan(QT_MINOR_VERSION,7) {
	error("Cobra REQUIRED atleast QT 4.7.1")
}

equals(QT_MAJOR_VERSION,4) : lessThan(QT_MINOR_VERSION,8) {
	message("Building Cobra WITHOUT Audio Support.")
	DEFINES += NO_AUDIO
} else : equals(QT_MINOR_VERSION,8) : lessThan(QT_PATCH_VERSION,1) {
	message("Building Cobra WITHOUT Audio Support.")
	DEFINES += NO_AUDIO
} else {
	message("Building Cobra WITH Audio Support.")
	QT += phonon
}

SOURCES = src/main.cc \
    src/mainwindow.cc \
    src/debug.cc \
    src/net/handler.cc \
    src/net/thread.cc \
    src/net/connection.cc \
    src/event/state.cc \
    src/event/event.cc \
    src/event/chat.cc \
    src/filevalidator.cc \
    src/treewidget.cc \
    src/event/auth.cc \
    src/preferences.cc \
    src/event/transfer.cc \
    src/event/transfer_controller.cc \
    src/transfersdlg.cc \
    src/clip.cc \
    src/clipdialog.cc \
    src/event/update_clip.cc \
    src/timeline.cc \
    src/event/update_timeline.cc \
    src/timelinetree.cc \
    src/timelinedialog.cc \
    src/markerdialog.cc \
    src/event/play.cc

INCLUDEPATH = include

# Directories

HEADERS += \
    include/mainwindow.h \
    include/debug.h \
    include/treewidget.h \
    include/net.h \
    include/event.h \
    include/filevalidator.h \
    include/preferences.h \
    include/transfersdlg.h \
    include/clip.h \
    include/transfer.h \
    include/clipdialog.h \
    include/timeline.h \
    include/timelinetree.h \
    include/timelinedialog.h \
    include/markerdialog.h \
    include/play_event.h \
    include/timeline_event.h \
    include/clip_event.h \
    include/chat_event.h \
    include/auth_event.h \
    include/state_event.h

FORMS += \
    resources/mainwindow.ui \
    resources/mainwindow_mdi.ui \
    resources/preferences.ui \
    resources/transfersdlg.ui \
    resources/clipdialog.ui \
    resources/mainwindow_docks.ui \
    resources/timelinedialog.ui \
    resources/markerdialog.ui

TRANSLATIONS = resources/translations/cobratr_cn.ts

RESOURCES += \
    resources/cobra.qrc

QMAKE_CXXFLAGS += -fprofile-arcs -ftest-coverage
QMAKE_LDFLAGS += -fprofile-arcs -ftest-coverage
LIBS += -lgcov
