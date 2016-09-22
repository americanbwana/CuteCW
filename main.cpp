#include <QApplication>
#include <QTranslator>
#include <QLibraryInfo>
#include <QSettings>
#include <QDebug>
#include <QDir>

#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

#ifdef PORTABLE_BUILD
    QSettings settings(QDir::currentPath() + "/cutecw.cfg", QSettings::IniFormat);
    qDebug() << "Using portable config from" << QDir::currentPath() << "/cutecw.cfg";
#else
    QSettings settings("WS6Z", "cutecw");
    qDebug() << "Using native config";
#endif

    QString locale = settings.value("Lang", "en").toString();

    // Qt5 strings
    QTranslator qtTranslator;
    qtTranslator.load("qt_" + locale, QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    a.installTranslator(&qtTranslator);

    // CuteCW strings
    QTranslator cutecwTranslator;
    cutecwTranslator.load("cutecw_" + locale);
    a.installTranslator(&cutecwTranslator);

    MainWindow w;
#if defined(Q_WS_S60)
    w.showMaximized();
#else
    w.show();
#endif

    return a.exec();
}
