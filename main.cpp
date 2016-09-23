#include <QApplication>
#include <QTranslator>
#include <QLibraryInfo>
#include <QSettings>
#include <QDebug>
#include <QDir>

#include "MainWindow.h"

QString findTranslations() {
    QStringList searchDirs;

#if defined(Q_OS_X11) || defined(Q_OS_LINUX)
    searchDirs.append("/usr/share/cutecw/");
#elif defined(Q_OS_WIN)
    searchDirs.append(QCoreApplication::applicationDirPath());
#endif

    // TODO: Mac OSX Bundle search

    // fallback, insert CWD
    searchDirs.append(QDir::currentPath());

    foreach(const QString &fn, searchDirs) {
        QDir fic(fn);
        if (fic.exists("cutecw_en.qm") && fic.isReadable()) {
            qInfo() << "found translations in" << fn;
            return fn;
        }
    }
    // fallback returns CWD
    qCritical() << "unable to found translations in paths, fallback to current CWD even if not fond in it";
    return QDir::currentPath();
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Set qDebug pattern
    qSetMessagePattern("[%{time}][%{if-debug}D%{endif}%{if-info}I%{endif}%{if-warning}W%{endif}%{if-critical}C%{endif}%{if-fatal}F%{endif}][%{file}:%{line} (%{function})] %{message}");

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
    QString translationsDir = findTranslations();
    cutecwTranslator.load("cutecw_" + locale, translationsDir);
    a.installTranslator(&cutecwTranslator);

    MainWindow w;
#if defined(Q_WS_S60)
    w.showMaximized();
#else
    w.show();
#endif

    return a.exec();
}
