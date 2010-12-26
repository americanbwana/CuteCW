#include <qdebug.h>

#include "modes/games/GroupGame.h"
#include "modes/QModeStart.h"

#include <QtGui/QFormLayout>
#include <QtGui/QSpinBox>

GroupGame::GroupGame(Morse *parent, Ui::MainWindow *ui) :
  GroupingMode(parent, ui), MCountGameMode(), m_scores("Group Accuracy Game")
{
    connect(this, SIGNAL(groupEntered(int, int)),
            this, SLOT(groupGuessed(int, int)));
    m_WPM = m_morse->currentWPMGoal();
}

void GroupGame::switchToMode() {
    GroupingMode::switchToMode();
    m_ui->clearTraining->hide();
    m_ui->modeMenu->setText(tr("Grouping Game"));
    m_ui->helpBar->setText(tr("<font color=\"green\">Type the groups of characters you hear ASAP.</font>"));
    m_ui->changeSequence->hide();
}

void GroupGame::play()
{
    clear();
    startGame();

    QModeStart startInfo(0, tr("Grouping Game"));
    QVBoxLayout *layout = startInfo.mainLayout();

    QFormLayout form;
    layout->addLayout(&form);

    QSpinBox groupLength;
    groupLength.setValue(1);
    form.addRow(tr("Starting Group Length:"), &groupLength);

    QSpinBox WPM;
    WPM.setValue(m_WPM);
    form.addRow(tr("Starting WPM:"), &WPM);

    if (startInfo.exec() == QDialog::Accepted) {
        m_goodGuesses = GROUPLENGTH_WEIGHT * (groupLength.value() - 1);
        m_WPM = WPM.value();
        m_morse->createTones(m_WPM);
        startNextGroup();
    }
}

void GroupGame::gameOver()
{
  pause();
  m_scores.addScore("", score());
}

void GroupGame::groupGuessed(int right, int total)
{
    nextTurn(m_WPM * right * 10 * ((total==right)?2:1));
    if (right == total)
        m_WPM++;
    else
        m_WPM--;
    m_morse->createTones(m_WPM);
}

void GroupGame::setSequenceText()
{
  m_morse->m_sequenceLabel->setText(tr("current length = %1, WPM=%2, score = %3").arg(m_groupLength).arg(m_WPM).arg(score()));
}

QString GroupGame::helpText()
{
    return tr("Characters in the chosen training sequence will be keyed starting with a single character.  "
              "Type in the characters you hear in the group.  Your results good or bad will be displayed "
              "on the screen in green/red colors indicating good/bad guesses."
              "<p>As you guess more and more groups correctly the number of characters in the group will slowly go upward.");
}

