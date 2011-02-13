#include "FrontPage.h"
#include "Morse.h"

#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QPushButton>
#include <QtCore/QSignalMapper>

FrontPage::FrontPage(Morse *parent, Ui::MainWindow *ui)
    : MorseMode(parent, ui)
{
}

void FrontPage::switchToMode()
{
    m_ui->helpBar->setText("<font color=\"green\">Selecting a starting place from the buttons below</font>");

    m_ui->letter->hide();
    m_ui->play->hide();
    m_ui->changeSequence->hide();

    m_ui->sequence->hide();
    clearLayout(m_ui->horizontalLayout_3);
    clearLayout(m_ui->horizontalLayout_4);
    clearLayout(m_ui->horizontalLayout_5);

    setupWidgets();
}

void FrontPage::setupWidgets() {
    QSignalMapper *mapper = new QSignalMapper(m_ui->forModes);
    QVBoxLayout *topVBox = new QVBoxLayout();
    QHBoxLayout *mainHBox = new QHBoxLayout();
    topVBox->setStretchFactor(mainHBox, 5);
    topVBox->addLayout(mainHBox);

    QVBoxLayout *leftBox = new QVBoxLayout();
    mainHBox->addLayout(leftBox);
    QVBoxLayout *rightBox = new QVBoxLayout();
    mainHBox->addLayout(rightBox);

    QPushButton *button;
    QList<Morse::TrainingMode> leftContents, rightContents;
    leftContents << Morse::TRAIN << Morse::SPEEDTRAIN << Morse::WORDS << Morse::GROUPS;
    rightContents << Morse::PLAY << Morse::READ << Morse::WORDGAME << Morse::GROUPGAME;

    foreach(Morse::TrainingMode themode, leftContents) {
        button = new QPushButton(m_morse->getMode(themode)->name());
        QSizePolicy policy = button->sizePolicy();
        policy.setVerticalPolicy(QSizePolicy::Maximum);
        button->setSizePolicy(policy);
        leftBox->addWidget(button);
        connect(button, SIGNAL(clicked()), mapper, SLOT(map()));
        mapper->setMapping(button, (int) themode);
    }

    foreach(Morse::TrainingMode themode, rightContents) {
        button = new QPushButton(m_morse->getMode(themode)->name());
        rightBox->addWidget(button);
        connect(button, SIGNAL(clicked()), mapper, SLOT(map()));
        mapper->setMapping(button, (int) themode);
    }

    connect(mapper, SIGNAL(mapped(int)), m_morse, SLOT(switchMode(int)));

    m_ui->forModes->addLayout(topVBox);
    m_ui->forModes->setStretchFactor(topVBox, 5);
}


QString FrontPage::helpText()
{
    return tr("<p>From this screen you can access all the other modes.  This is just a starting screen.  "
              "<p>If you want to learn morse code, consider following the steps in the left hand panel.  "
              "Then, to continue your training and/or have some fun in the process, try the modes in the right hand panel."
              "<p>once you enter a mode, use the menus to navegate to other modes.</p>");
}

QString FrontPage::name()
{
    return tr("Home Page");
}
