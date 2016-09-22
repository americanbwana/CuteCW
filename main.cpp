#include <QApplication>
#include <QTranslator>
#include <QLibraryInfo>
#include <QSettings>

#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QSettings settings("WS6Z", "qtcw");

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
