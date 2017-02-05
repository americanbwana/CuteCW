#include <QDebug>
#include <QMenu>
#include <QStandardPaths>
#include <QDir>
#include <QFile>

#include "WordTrainingMode.h"

WordTrainingMode::WordTrainingMode(Morse *parent, Ui::MainWindow *ui)
    : MorseMode(parent, ui)
{
    setupWords();
}


void WordTrainingMode::setupWords() {
    QStringList searchDirs;

    // Build the search list, either PORTABLE or not and adding "/words/wordtraining" to it's end.
#ifdef PORTABLE_BUILD
    searchDirs.append(QDir::currentPath() + "/words/wordtraining")
#else
    QStringList standardPaths = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);
    foreach(const QString &dn, standardPaths) {
        searchDirs.append(dn + "/words/wordtraining");
    }
#endif
    qDebug() << "Word Training search paths: " << searchDirs;

    QString wordsDirectory;

    QStringList nameFilter;
    nameFilter << "*.lst.txt";

    // Now iterate over each words path to find the right one
    foreach (const QString &dn, searchDirs) {
        QDir dir = QDir(dn);
        if (!dir.exists()) {
            continue; // directory doesn't exists
        }

        QStringList filesList = dir.entryList(nameFilter, QDir::Files | QDir::Readable, QDir::Name);
        qDebug() << filesList;

        if (filesList.length() >= 1) {
            wordsDirectory = dn;
            break; // found the first one with words, stop here
        }
    }

    // Populate structures with words
    QDir dir = QDir(wordsDirectory);
    QStringList files = dir.entryList(nameFilter, QDir::Files | QDir::Readable, QDir:: Name);
    foreach (const QString &fn, files) {
        QFile file(wordsDirectory + "/" + fn);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            continue;

        // Process first line
        QByteArray name = file.readLine();
        words[name] = new QStringList;

        while (!file.atEnd()) {
            QByteArray line = file.readLine();
            words[name]->append(line);
        }
    }

    // Set the default wordsNumber as first list name in words
    m_wordsListName = words.firstKey();
}

void WordTrainingMode::setupWordsMenu() {

    m_wordSignalMapper = new QSignalMapper();
    QMenu *modeMenu = new QMenu(m_ui->changeWords);
    m_ui->changeWords->setMenu(modeMenu);

    foreach (QString list, words.keys()) {
        QAction *action = modeMenu->addAction(list);
        connect(action, SIGNAL(triggered()), m_wordSignalMapper, SLOT(map()));
        m_wordSignalMapper->setMapping(action, list);
    }

    connect(m_wordSignalMapper, SIGNAL(mapped(int)), this, SLOT(switchWords(int)));
}

void WordTrainingMode::switchToMode() {
    setupSequenceLayout();

    m_ui->letter->show();
    m_ui->changeWords->show();
    m_ui->helpBar->setText(tr("<font color=\"green\">Enter the word you hear and hit enter.</font>"));
    m_ui->play->show();

    setupWordsMenu();
    setupKeyWidgets("abcdefghijklmnopqrstuvwxyz");
    setSequenceText();
}

void WordTrainingMode::switchWords(int sequence) {
    m_wordsListName = (QString) sequence;
    qDebug() << "switching to: " << m_wordsListName;
}

void WordTrainingMode::play() {
    enterPressed();
}

bool WordTrainingMode::enterPressed() {
    m_wordNumber = qrand()%(m_maxWord);
    m_morse->add((*(words[m_wordsListName]))[m_wordNumber]);
    m_morse->maybePlaySequence(true);
    m_enteredWord = "";
    m_ui->letter->setText("");
    m_wordWasGood = true;
    m_rightCount = 0;
    setSequenceText();
    return true;
}

void WordTrainingMode::setSequenceText()
{
    m_sequenceLabel->setText(tr("Words: %1/%2").arg(m_maxWord).arg(words[m_wordsListName]->length()));
}

void WordTrainingMode::handleKeyPress(QChar letter) {
    if (runningMode() != RUNNING)
        return;
    if (letter == '\r') {
        enterPressed();
        return;
    }

    if ((*(words[m_wordsListName]))[m_wordNumber].length() == m_enteredWord.length()) // they already hit the length previously
        return;

    if ((*(words[m_wordsListName]))[m_wordNumber][m_enteredWord.length()] == letter) {
        m_ui->letter->setText(m_ui->letter->text() + "<font color=\"green\">" + letter + "<font>");
        m_rightCount++;
    } else {
        m_ui->letter->setText(m_ui->letter->text() + "<font color=\"red\">" + letter + "<font>");
        m_wordWasGood = false;
    }
    m_enteredWord.append(letter);
    if ((*(words[m_wordsListName]))[m_wordNumber].length() == m_enteredWord.length()) {
        if (m_wordWasGood) {
            m_ui->letter->setText(tr("%1 - <font color=\"green\">GOOD</font>").arg(m_ui->letter->text()));
            if (m_maxWord < 10)
                m_maxWord += 2;
            else
                m_maxWord += 1;
            if (m_maxWord > (*(words[m_wordsListName])).count())
                m_maxWord = (*(words[m_wordsListName])).count();

        } else {
            m_ui->letter->setText(tr("%1 - <font color=\"red\">FAIL (%2)</font>").arg(m_ui->letter->text()).arg((*(words[m_wordsListName]))[m_wordNumber]));
            if (m_maxWord > 1)
                m_maxWord--;
        }
        emit groupEntered(m_rightCount, m_enteredWord.length());
    }
}

QString WordTrainingMode::helpText()
{
    return
            tr("Most words that you read or hear are from a very small set of words.  For example, 33% of the most common words fall into 100 simple words.  "
              "If you memorize these words so you can regonize them not as individual letters but as entire words you'll greatly increase your listening rate.  "
              "<p>As you hear words, type them in and hit enter when done.  After hitting enter the next word will be played.  "
               "As you begin the number of words selected from will be small but will increase quickly for every right answer (and decrease for every wrong)."
               "<p>Once you get good at the words in this group, try switching to the <i>Word Game</i> mode and see what score you can achieve!");
}

QString WordTrainingMode::name() {
    return tr("Word Training");
}

QString WordTrainingMode::rawName() {
    return "Word Training";
}

QString WordTrainingMode::icon()
{
    return ":/icons/64x64/words.png";
}

void WordTrainingMode::loadSettings(QSettings &settings)
{
    QString prefix = rawName();
    //m_wordsListName = (QString) settings.value(prefix + "/wordsNumber",  int(N100)).toInt();
    m_maxWord     =            settings.value(prefix + "/maxWord",      2).toInt();
}

void WordTrainingMode::saveSettings(QSettings &settings)
{
    QString prefix = rawName();
    settings.setValue(prefix + "/wordsNumber", m_wordsListName);
    settings.setValue(prefix + "/maxWord",     m_maxWord);
}
