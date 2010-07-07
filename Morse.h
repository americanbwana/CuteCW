#ifndef MORSE_H
#define MORSE_H

#include <QtCore/QString>
#include <QtMultimedia/QAudioOutput>
#include <QtCore/QMap>
#include <QtCore/QList>
#include <QtGui/QLabel>
#include <QtCore/QTime>

#include "Generator.h"
#include "MorseStat.h"

#define KOCH_GROUP "kmuresnaptlwi.jz=foy,vg5/q92h38b?47c1d60x"

#include "ui_MainWindow.h"
#include "ui_Prefs.h"

class MainWindow;

class Morse : public QObject
{
public:
    Q_OBJECT

public:
    Morse();
    Morse(MainWindow *parent, QAudioOutput *output, Ui::MainWindow *ui);

    enum ditdah{ DIT, DAH };

    enum mode { PLAY, TRAIN, TEST };
    enum playingmode { STOPPED, PLAYING };
    enum badLetterWeighting { LOW = 1, HIGH = 2 };

    void clearList();
    void add(Generator *nextsound);
    void add(QChar c, bool addpause = true);
    void addAndPlayIt(QChar c);
    void createTones(float ditSecs, int dahMult = 3, int pauseMult = 1, int letterPauseMult = 3, int spaceMult = 7);
    MorseStat *getStat(const QChar &key);
    void startNextTrainingKey();
    int  msToWPM(float ms);
    int  msToPauseWPM(float ms);

    void setStatus(const QString &status);
    void setSequence(const QString &sequence, int currentlyAt);

    Generator *dit();
    Generator *dah();
    Generator *pause();
    Generator *letterPause();
    Generator *space();

public slots:
    void playSequence();
    void maybePlaySequence();
    void audioFinished(QAudio::State state);
    void keyPressed(QString newtext);
    void keyPressed(QChar key);
    void switchMode(int newMode);
    void prefsButton();

    void saveSettings();
    void loadSettings();

private:
    MainWindow                      *m_parent;
    QAudioOutput                    *m_audioOutput;
    float                           m_ditSecs;
    Generator                       *m_dit, *m_dah, *m_space, *m_pause, *m_letterPause;
    Generator                       *m_playBuffer;
    QMap<QChar, QList<ditdah> *>    code;
    playingmode                     m_playingMode;
    mode                            m_gameMode;
    int                             m_currentWPMGoal;
    int                             m_currentWPMAccept;
    QMap<QChar, MorseStat *>        m_stats;
    QChar                           m_lastKey;
    QTime                           m_lastTime;
    QString                         m_trainingSequence;
    QLabel                          *m_statusBar;
    QLabel                          *m_sequenceLabel;
    Ui::MainWindow                  *m_ui;
    badLetterWeighting               m_badLetterWeighting;

};

#endif // MORSE_H
