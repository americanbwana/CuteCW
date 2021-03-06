
#include "Morse.h"

#include <QtCore/QSettings>
#include <QMenu>
#include <QMenuBar>
#include <QSizePolicy>
#include <qdebug.h>

#include "MainWindow.h"
#include "Prefs.h"

#include "modes/FrontPage.h"
#include "modes/PlayMode.h"
#include "modes/LetterTrainingMode.h"
#include "modes/SpeedTrainingMode.h"
#include "modes/WordTrainingMode.h"
#include "modes/KeyTraining.h"
#include "modes/ReadMode.h"
#include "modes/GroupingMode.h"
#include "modes/games/WordGame.h"
#include "modes/games/GroupGame.h"
#include "modes/games/KeyInvaders.h"

#define WPMGOAL   20
#define WPMACCEPT 2

Morse::Morse()
    : QObject(), m_parent(0), m_audioOutput(),
      m_dahMult(3), m_pauseMult(1), m_letterPauseMult(3), m_spaceMult(7),
      m_dit(0), m_dah(0), m_space(0), m_pause(0), m_letterPause(0), m_playBuffer(0), m_playingMode(STOPPED), m_gameMode(PLAY),
    m_currentWPMGoal(WPMGOAL), m_currentWPMAccept(WPMACCEPT), m_ui(0), m_tone(DEFAULT_TONE), m_leadInPause(0,0), m_signalMapper(new QSignalMapper(this))
{
    qDebug() << "new morse";
    m_modes.insert(PLAY, new PlayMode(this, m_ui));
#include "morse_code.h"
}

Morse::Morse(MainWindow *parent, QAudioOutput *output, Ui::MainWindow *ui)
    : QObject(parent), m_parent(parent), m_audioOutput(output),
      m_dahMult(3), m_pauseMult(1), m_letterPauseMult(3), m_spaceMult(7),
      m_dit(0), m_dah(0), m_space(0), m_pause(0), m_letterPause(0), m_playBuffer(0), m_playingMode(STOPPED), m_gameMode(PLAY),
      m_currentWPMGoal(WPMGOAL), m_currentWPMAccept(WPMACCEPT),
      m_ui(ui), m_tone(DEFAULT_TONE), m_leadInPause(0,0), m_signalMapper(new QSignalMapper(this))
{

    qDebug() << "new morse2";

#include "morse_code.h"

    createTones(WPMGOAL);
    qsrand(QTime::currentTime().msec());

    qDebug() << "original buffer size: " << m_audioOutput->bufferSize();
    m_audioOutput->setBufferSize(qMin(m_audioOutput->bufferSize() * 4, 1024*64));

    m_modes.insert(FRONTPAGE, new FrontPage(this, m_ui));
    m_modes.insert(PLAY, new PlayMode(this, m_ui));
    m_modes.insert(TRAIN, new LetterTrainingMode(this, m_ui));
    m_modes.insert(SPEEDTRAIN, new SpeedTrainingMode(this, m_ui));
    m_modes.insert(WORDS, new WordTrainingMode(this, m_ui));
    m_modes.insert(GROUPS, new GroupingMode(this, m_ui));
    m_modes.insert(KEYING, new KeyTraining(this, m_ui));
    m_modes.insert(WORDGAME, new WordGame(this, m_ui));
    m_modes.insert(GROUPGAME, new GroupGame(this, m_ui));
    m_modes.insert(READ, new ReadMode(this, m_ui));
    m_modes.insert(KEYINVADERS, new KeyInvaders(this, m_ui));

    loadSettings();

    switchMode(Morse::FRONTPAGE);
}

MorseMode *Morse::getMode(TrainingMode which) const {
    return m_modes[which];
}

void Morse::prefsButton() {
    Prefs prefs(this);
    prefs.setWindowTitle(tr("CuteCW preferences"));
    prefs.exec();
}

void Morse::aboutButton() {
    Ui::AboutDialog about;
    QDialog *dialog = new QDialog(m_parent);
    about.setupUi(dialog);
    about.helpicon->setPixmap(QPixmap(":/icons/64x64/cutecw.png"));
#ifdef SMALL_DEVICE
    dialog->setAttribute(Qt::WA_Maemo5StackedWindow);
#endif
    dialog->exec();
}

QMenuBar *
Morse::menuBar()
{
    return m_parent->menuBar();
}

void Morse::saveSettings() {
    qDebug() << "saving!";

#ifdef PORTABLE_BUILD
    QSettings settings(QDir::currentPath() + "/cutecw.cfg", QSettings::IniFormat);
    qDebug() << "Using portable config from" << QDir::currentPath() << "/cutecw.cfg";
#else
    QSettings settings("WS6Z", "cutecw");
    qDebug() << "Using native config";
#endif

    settings.setValue("WPM/Goal", m_currentWPMGoal);
    settings.setValue("WPM/Accept", m_currentWPMAccept);
    settings.setValue("Tone", m_tone);
    settings.setValue("StartSound", m_startSound);
    settings.setValue("Lang", m_lang);
    settings.setValue("StartSoundStr", m_startSoundStr);
    //settings.setValue("LetterWeighting", int(m_badLetterWeighting));

    for(int i = PLAY; i <= maximumTrainingMode; i++) {
        m_modes[(TrainingMode) i]->saveSettings(settings);
    }
}

void Morse::loadSettings() {
#ifdef PORTABLE_BUILD
    QSettings settings(QDir::currentPath() + "/cutecw.cfg", QSettings::IniFormat);
    qDebug() << "Using portable config from" << QDir::currentPath() << "/cutecw.cfg";
#else
    QSettings settings("WS6Z", "cutecw");
    qDebug() << "Using native config";
#endif

    m_currentWPMGoal = settings.value("WPM/Goal", WPMGOAL).toInt();
    m_currentWPMAccept = settings.value("WPM/Accept", WPMACCEPT).toInt();
    //m_badLetterWeighting = (BadLetterWeighting) settings.value("LetterWeighting", HIGH).toInt();
    m_tone = settings.value("Tone", DEFAULT_TONE).toInt();
    m_startSound = settings.value("StartSound", true).toBool();
    m_lang = settings.value("Lang", "en").toString();
    m_startSoundStr = settings.value("StartSoundStr", "de WS6Z").toString();

    for(int i = PLAY; i <= maximumTrainingMode; i++) {
        if (! m_modes.contains((TrainingMode) i))
            continue;
        m_modes[(TrainingMode) i]->loadSettings(settings);
    }
    createTones(m_currentWPMGoal);  
}

void Morse::clearStatsButton() {
    m_modes[m_gameMode]->clear();
}

void
Morse::playSequence()
{
    m_playBuffer->restartData();
    m_playBuffer->start();
    m_playingMode = PLAYING;

    /* Same crap bug like on Morse::setAudioMode
     */
    if (m_audioOutput->error() != QAudio::NoError) {
        // on OS X, it's every time the audio pauses
        m_audioOutput = m_parent->createAudioOutput();
    }
    
    
    m_audioOutput->start(m_playBuffer);
    return;
}

QTime Morse::sequenceTime() {
    return m_playBuffer->timeLeft();
}

QTime Morse::maybePlaySequence(bool addPause) {
    if (m_playingMode == STOPPED || m_playingMode == PAUSED ||
        m_audioOutput->state() != QAudio::ActiveState) {
        m_playBuffer->restartData();
        QTime playTime = sequenceTime();
        if (addPause)
            add(pause());
        playSequence();

        return QTime::currentTime().addSecs(playTime.second()).addMSecs(playTime.msec());
    }

    qDebug() << "playing mode" << m_playingMode << "returning QTime(0,0,0) [enum AudioMode { STOPPED, PLAYING, PAUSED };]";
    /* Try to fix the multiple-keys-entered bug by setting audio to any mode other than playing */
    // LetterTrainingMode.cpp L82 fix used instead
    //setAudioMode(PAUSED);
    return QTime(0,0,0);
}


void Morse::keyPressed(QString newtext) {
    QChar newletter = newtext.at(newtext.length()-1).toLower();
    qDebug() << "*** user pressed: " << newletter;
    keyPressed(newletter);
}

void Morse::keyPressed(QChar newletter) {
    m_modes[m_gameMode]->handleKeyPress(newletter);
}

bool Morse::enterPressed() {
    return m_modes[m_gameMode]->enterPressed();
}

void Morse::keyReleased(QString newtext) {
    QChar newletter = newtext.at(newtext.length()-1).toLower();
    qDebug() << "*** user pressed: " << newletter;
    keyReleased(newletter);
}

void Morse::keyReleased(QChar newletter) {
    m_modes[m_gameMode]->handleKeyRelease(newletter);
}

Morse::AudioMode Morse::audioMode() {
    return m_playingMode;
}

void Morse::playButton() {
    m_modes[m_gameMode]->playButton();
}

void Morse::setAudioMode(AudioMode newmode) {
    m_playingMode = newmode;
    /* Like in Morse::playSequence we have the MAC OSX bug
     * If we call m_audioOutput->stop() we will get a crash
     * Instead, call a stop on the playBuffer and let things goes on
     */
#ifdef Q_OS_DARWIN
        m_playBuffer->stop();
#else
        m_audioOutput->stop();
#endif
}

void Morse::pauseAudio() {
    setAudioMode(PAUSED);
}

void Morse::playAudio() {
    setAudioMode(PLAYING);
}

Morse::TrainingMode Morse::trainingMode() {
    return m_gameMode;
}

Morse::BadLetterWeighting Morse::badLetterWeighting() {
    return m_badLetterWeighting;
}

void
Morse::audioStateChanged(QAudio::State newState)
{
    if (newState == QAudio::IdleState)
        m_modes[m_gameMode]->audioFinished();
}

void Morse::switchMode(int newmode) {
    qDebug() << "switch to:" << newmode;
    m_playBuffer->stop();

    m_modes[m_gameMode]->switchFromYou();
    m_gameMode = (Morse::TrainingMode) newmode;

    setupWidgets();
    m_modes[(TrainingMode) newmode]->switchToYou();
}

//
// HERE and below is tone generation and sequence playing
//

QTime Morse::addAndPlayIt(QChar c) {
    if (m_playingMode == STOPPED || m_playingMode == PAUSED) {
        clearList();
        add(pause());
    }
    add(c, false);
    add(m_letterPause);
    return maybePlaySequence();
}

QTime Morse::playIt(QChar c, bool addLeadInPause) {
    clearList();
    if (addLeadInPause)
        add(pause());  // allows audio device to kick in (otherwise distortion can occur)
    m_leadInPause = sequenceTime();
    add(c, false); // add the pause in maybePlaySequence so it can calculate the sequence end

    /* https://github.com/hardaker/CuteCW/issues/12
     * Add a letter pause twice so audio does not stop early (cutting of
     * the last dit or dah).
     * This is a workaround and we should probably just let the audio
     * run a little longer.
     */
    add(m_letterPause);
    //add(m_letterPause);

    return maybePlaySequence(true);
}

void
Morse::clearList()
{
    m_playBuffer->clearBuffer();
}

void
Morse::add(Generator *nextSound)
{
    m_playBuffer->appendDataFrom(nextSound);
}

void
Morse::add(QChar c, bool addpause)
{
    c = c.toLower();
    if (! code.contains(c))
        return;

    QList<ditdah>::iterator iter;
    QList<ditdah>::iterator endat = code[c]->end();

    bool lastWasPause = false;
    for(iter = code[c]->begin(); iter != endat; iter++)
    {
        lastWasPause = false;
        switch (*iter) {
        case DIT:
            add(m_dit);
            break;
        case DAH:
            add(m_dah);
            break;
        case PAUSE:
            add(m_pause);
            lastWasPause = true;
            break;
        case SPACE:
            add(m_space);
            lastWasPause = true;
            break;
        default:
            qWarning() << "error: illegal morse type added";
        }
        add(m_pause);
    }
    if (addpause && !lastWasPause) {
        add(m_letterPause);
    }
}

void Morse::add(const QString &textToAdd) {
    QString::const_iterator letter;
    QString::const_iterator lastLetter = textToAdd.end();

    clearList();
    for (letter = textToAdd.begin(); letter != lastLetter; ++letter) {
        add(*letter, true);
    }
}

void
Morse::createTones(int wpm, int spacewpm, int letterspacewpm)
{
    if (spacewpm == -1)
        spacewpm = wpm;
    if (letterspacewpm == -1)
        letterspacewpm = spacewpm;
    createSpacedTones(float(60.0/float(wpm*50.0)), float(float(m_spaceMult) * float(60.0/float(spacewpm*50.0))),
                float(float(m_letterPauseMult) * float(60.0/float(letterspacewpm*50.0))));
}

void
Morse::_createTones()
{
    if (m_dit)
        delete m_dit;
    m_dit = new Generator(m_ditSecs, m_tone);
    m_dit->start();

    if (m_dah)
        delete m_dah;
    m_dah = new Generator(m_dahSecs, m_tone);
    m_dah->start();

    if (m_pause)
        delete m_pause;
    m_pause = new Generator(m_pauseSecs, 0);
    m_pause->start();

    if (m_letterPause)
        delete m_letterPause;
    m_letterPause = new Generator(m_letterPauseSecs, 0);
    m_letterPause->start();

    if (m_space)
        delete m_space;
    m_space = new Generator(m_spaceSecs, 0);
    m_space->start();

    if (!m_playBuffer) {
        m_playBuffer = new Generator(m_pause);
        m_playBuffer->start();
    }

    connect(m_audioOutput, SIGNAL(stateChanged(QAudio::State)), this, SLOT(audioStateChanged(QAudio::State)));
}

void
Morse::createTones(float ditSecs, int dahMult, int pauseMult, int letterPauseMult, int spaceMult)
{
    m_ditSecs = ditSecs;
    m_dahSecs = ditSecs * dahMult;
    m_pauseSecs = ditSecs * pauseMult;
    m_letterPauseSecs = ditSecs * letterPauseMult;
    m_spaceSecs = ditSecs * spaceMult;

    m_dahMult = dahMult;
    m_pauseMult = pauseMult;
    m_letterPauseMult = letterPauseMult;
    m_spaceMult = spaceMult;

    _createTones();
}

void
Morse::createSpacedTones(float ditSecs, float spaceSecs, float letterSpaceSecs)
{
    m_ditSecs = ditSecs;
    m_spaceSecs = spaceSecs;
    m_letterPauseSecs = letterSpaceSecs;
    m_dahSecs = ditSecs * m_dahMult;
    m_pauseSecs = ditSecs * m_pauseMult;
    _createTones();
}

int Morse::currentWPMAccept() {
    return m_currentWPMAccept;
}

int Morse::currentWPMGoal() {
    return m_currentWPMGoal;
}

Generator *
Morse::dit()
{
    return m_dit;
}

Generator *
Morse::dah()
{
    return m_dah;
}

Generator *
Morse::pause()
{
    return m_pause;
}

Generator *
Morse::letterPause()
{
    return m_letterPause;
}

Generator *
Morse::space()
{
    return m_space;
}

float
Morse::ditSecsF()
{
    return m_ditSecs;
}

void Morse::setupWidgets()
{
    QWidget *theMainThing = m_ui->centralWidget = new QWidget(m_parent);
    QVBoxLayout *topvbox = m_ui->verticalLayout = new QVBoxLayout(theMainThing);

    QHBoxLayout *buttonHBox = m_ui->horizontalLayout = new QHBoxLayout(theMainThing);
    topvbox->addLayout(buttonHBox);

    setupTopButtons(buttonHBox);
    topvbox->addWidget(m_ui->helpBar = new QLabel("help"));
    setupWPMLayout(topvbox, theMainThing);
    topvbox->addLayout(m_ui->forModes = new QHBoxLayout(theMainThing));
    topvbox->setStretchFactor(m_ui->forModes, 10);

    setupConnections();

    m_parent->setCentralWidget(theMainThing);
}


void Morse::setupWPMLayout(QVBoxLayout *parentLayout, QWidget *theMainThing)
{
    QHBoxLayout *WPMLayout = new QHBoxLayout(theMainThing);
    parentLayout->addLayout(WPMLayout);

    WPMLayout->addWidget(m_ui->letter = new QLabel(""));
    WPMLayout->addWidget(m_ui->WPM = new QLabel(""));

    QFont font = m_ui->WPM->font();
    font.setPointSize(font.pointSize() * 2); // A cheap way of getting bigger regardless of the default stylee
    font.setBold(true);
    m_ui->WPM->setFont(font);

    font = m_ui->letter->font();
    font.setPointSize(font.pointSize() * 2);
    font.setBold(true);
    m_ui->letter->setFont(font);
}

void Morse::setupTopButtons(QLayout *parentLayout)
{
    QPushButton *button;
    QSizePolicy policy;
    QIcon homeIcon(":/icons/go-home.png");

    button = m_ui->homeButton = new QPushButton(homeIcon, tr("Home"));
    policy = button->sizePolicy();
    policy.setHorizontalPolicy(QSizePolicy::Fixed);
    button->setSizePolicy(policy);
    parentLayout->addWidget(button);
    connect(button, SIGNAL(clicked()), this, SLOT(goHome()));

    button = m_ui->play = new QPushButton(tr("Play"));
    parentLayout->addWidget(button);
    m_ui->play->setIcon(QIcon(":/icons/play.png"));

    button = m_ui->changeSequence = new QPushButton(tr("Sequence"));
    parentLayout->addWidget(button);

    button = m_ui->changeWords = new QPushButton(tr("Change Words"));
    parentLayout->addWidget(button);
}

void Morse::createModesMenuButton(QPushButton *modeButton) {
    // Create the "mode" menu
    QMenu *modeMenu = new QMenu(modeButton);
    modeButton->setMenu(modeMenu);

    createModesMenu(modeMenu);
}

void Morse::createModesMenu(QMenu *modeMenu) {

    QAction *action = modeMenu->addAction(m_modes[PLAY]->name());
    connect(action, SIGNAL(triggered()), m_signalMapper, SLOT(map()));
    m_signalMapper->setMapping(action, (int) Morse::PLAY);

    QMenu *trainingMenu = modeMenu->addMenu(tr("Training"));

    action = trainingMenu->addAction(m_modes[TRAIN]->name());
    connect(action, SIGNAL(triggered()), m_signalMapper, SLOT(map()));
    m_signalMapper->setMapping(action, (int) Morse::TRAIN);

    action = trainingMenu->addAction(m_modes[SPEEDTRAIN]->name());
    connect(action, SIGNAL(triggered()), m_signalMapper, SLOT(map()));
    m_signalMapper->setMapping(action, (int) Morse::SPEEDTRAIN);

    action = trainingMenu->addAction(m_modes[WORDS]->name());
    connect(action, SIGNAL(triggered()), m_signalMapper, SLOT(map()));
    m_signalMapper->setMapping(action, (int) Morse::WORDS);

    action = trainingMenu->addAction(m_modes[GROUPS]->name());
    connect(action, SIGNAL(triggered()), m_signalMapper, SLOT(map()));
    m_signalMapper->setMapping(action, (int) Morse::GROUPS);

    action = trainingMenu->addAction(m_modes[KEYING]->name());
    connect(action, SIGNAL(triggered()), m_signalMapper, SLOT(map()));
    m_signalMapper->setMapping(action, (int) Morse::KEYING);

    QMenu *gamesMenu = modeMenu->addMenu(tr("Games"));

    action = gamesMenu->addAction(m_modes[WORDGAME]->name());
    connect(action, SIGNAL(triggered()), m_signalMapper, SLOT(map()));
    m_signalMapper->setMapping(action, (int) Morse::WORDGAME);

    action = gamesMenu->addAction(m_modes[GROUPGAME]->name());
    connect(action, SIGNAL(triggered()), m_signalMapper, SLOT(map()));
    m_signalMapper->setMapping(action, (int) Morse::GROUPGAME);

    action = modeMenu->addAction(m_modes[READ]->name());
    connect(action, SIGNAL(triggered()), m_signalMapper, SLOT(map()));
    m_signalMapper->setMapping(action, (int) Morse::READ);

    connect(m_signalMapper, SIGNAL(mapped(int)), this, SLOT(switchMode(int)));
#ifdef SMALL_DEVICE
    m_ui->modeMenu->setText(m_modes[PLAY]->name());
#endif
}

void Morse::setupConnections()
{
    connect(m_ui->play, SIGNAL(clicked()), this, SLOT(playButton()));
}

MainWindow *Morse::parent()
{
    return m_parent;
}

void Morse::goHome()
{
    switchMode(FRONTPAGE);
}

int Morse::tone()
{
    return m_tone;
}

bool Morse::startSound() {
    return m_startSound;
}

QString Morse::lang() {
    return m_lang;
}

QString Morse::startSoundStr() {
    return m_startSoundStr;
}

void Morse::setWPMGoal(int wpmGoal)
{
    m_currentWPMGoal = wpmGoal;
}

void Morse::setWPMAccept(int wpmAccept)
{
    m_currentWPMAccept = wpmAccept;
}

void Morse::setTone(int tone)
{
    m_tone = tone;
}

void Morse::setStartSound(bool state) {
    m_startSound = state;
}

void Morse::setLang(QString lang) {
    m_lang = lang;
}

void Morse::setStartSoundStr(QString str) {
    m_startSoundStr = str;
}

float Morse::dahSecsF()
{
    return m_dahSecs;
}

float Morse::pauseSecsF()
{
    return m_pauseSecs;
}

float Morse::letterPauseSecsF()
{
    return m_letterPauseSecs;
}

float Morse::spaceSecsF()
{
    return m_spaceSecs;
}

const QList<Morse::ditdah> *Morse::getLetterCode(QChar letter) const
{
    return code[letter];
}

int Morse::ditSecsMS()
{
    return int(m_ditSecs * 1000.0);
}

int Morse::dahSecsMS()
{
    return int(m_dahSecs * 1000.0);
}

int Morse::pauseSecsMS()
{
    return int(m_pauseSecs * 1000.0);
}

int Morse::letterPauseSecsMS()
{
    return int(m_letterPauseSecs * 1000.0);
}

int Morse::spaceSecsMS()
{
    return int(m_spaceSecs * 1000.0);
}
