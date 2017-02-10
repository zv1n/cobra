/*******************************************************************************
Copyright (c) 2012, All rights reserved.
	Terry Meacham <terry.t.meacham@gmail.com>
	Andrew Hicks <andrew.t.hicks@gmail.com>
	Nick Huskinson <nhuskinson@gmail.com>
	Jiake Liu  <jiakeliu@gmail.com>
*******************************************************************************/

#include <QApplication>
#include <QTranslator>

#include "debug.h"
#include "mainwindow.h"
#include <QtNetwork/QSslSocket>
#include "net.h"
#include "clip.h"
#include "timeline.h"
/**
 * @fn void handleArgs(QApplication& app)
 *
 * Hack way of handling input args for debug runs...
 * For some reason Qt doesn't have a standard way like getopts... boo.
 */
void
handleArgs(QApplication& app)
{
    int level = LOW;
    QStringList strs = app.arguments();
    for (int x=0; x<strs.count(); x++) {
        QString lower = strs.at(x).toLower();
        if (!lower.compare("low")) {
            level = LOW;
        } else if (!lower.compare("med")) {
            level = MED;
        } else if (!lower.compare("high")) {
            level = HIGH;
        } else if (!lower.compare("crit")) {
            level = CRITICAL;
        } else if (!lower.compare("err")) {
            level = ERROR(level);
        }
    }
    set_debug_level(level);
}

int
main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    handleArgs(app);

    QApplication::setWindowIcon(QIcon(":images/cobra_main.png"));

    QSettings cobra_settings("./cobra.ini", QSettings::IniFormat);
    g_cobra_settings = &cobra_settings;

    QTranslator translator;

    if (!(g_cobra_settings->contains("language")))
        translator.load("");
    else
    {
        if (g_cobra_settings->value("language").toString() == "Simplified Chinese")
            translator.load(":translations/cobratr_cn");
        else if  (g_cobra_settings->value("language").toString() == "English")
            translator.load("");
    }

    app.installTranslator(&translator);


    MainWindow cobraMain;
    cobraMain.show();

    app.exec();

    return 0;
}
