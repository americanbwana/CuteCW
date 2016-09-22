#include "Prefs.h"
#include "Morse.h"

#include <QTabWidget>
#include <QFormLayout>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QMessageBox>

Prefs::Prefs(Morse *morse, QWidget *parent) :
    QDialog(parent, Qt::Window), m_morse(morse), m_oldTone(-1)
{
    QVBoxLayout *topLayout = new QVBoxLayout();
    QTabWidget *tabWidget = new QTabWidget();
    QWidget *genericPrefs = new QWidget();
    QFormLayout *genericForm = new QFormLayout();
    genericPrefs->setLayout(genericForm);

    m_WPMRate = new QSpinBox();
    m_WPMRate->setRange(1,40);
    m_WPMRate->setValue(m_morse->currentWPMGoal());
    genericForm->addRow(tr("WPM Goal and Play Rate"), m_WPMRate);

    m_tone = new QSpinBox();
    m_tone->setRange(100,1200);
    m_tone->setValue(m_morse->tone());
    genericForm->addRow(tr("CW Tone Frequency"), m_tone);

    QPushButton *button = new QPushButton(tr("Test"));
    genericForm->addRow("", button);
    connect(button, SIGNAL(clicked()), this, SLOT(testTone()));

    m_StartSound = new QCheckBox();
    if (m_morse->startSound())
        m_StartSound->setCheckState(Qt::Checked);
    else
        m_StartSound->setCheckState(Qt::Unchecked);
    genericForm->addRow(tr("Play startup sound"), m_StartSound);

    m_Lang = new QComboBox();
    m_Lang->addItem("English", QVariant("en"));
    m_Lang->addItem("Français", QVariant("fr"));

    int langIndex = m_Lang->findData(QVariant(m_morse->lang()));
    m_Lang->setCurrentIndex(langIndex);

    genericForm->addRow(tr("App language"), m_Lang);

    tabWidget->addTab(genericPrefs, tr("General"));

    topLayout->addWidget(tabWidget);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                                       | QDialogButtonBox::Cancel);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(ok()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(cancel()));
    topLayout->addWidget(buttonBox);

    // load all the mode preferences if they have some
    for(int which = Morse::TM_FIRST; which <= Morse::TM_LAST ; which++) {
        MorseMode *mode = m_morse->getMode(Morse::TrainingMode(which));
        QBoxLayout *modeLayout;
        if ((modeLayout = mode->getPrefsLayout()) != 0) {
            QWidget *modeWidget = new QWidget();
            modeWidget->setLayout(modeLayout);
            tabWidget->addTab(modeWidget, tr(qPrintable(mode->name())));
        }
    }

    setLayout(topLayout);
#ifdef SMALL_DEVICE
    setAttribute(Qt::WA_Maemo5StackedWindow);
#endif
    resize(800,440);
}

void Prefs::ok()
{
    m_morse->setTone(m_tone->value());
    m_morse->setWPMGoal(m_WPMRate->value());
    m_morse->setStartSound(m_StartSound->isChecked());

    QString newLang = m_Lang->itemData(m_Lang->currentIndex()).toString();
    if (m_morse->lang() != newLang)
        QMessageBox::information(this, tr("Language change"), tr("Please restart CuteCW to apply language change."));

    m_morse->setLang(newLang);
    m_morse->saveSettings();
    m_morse->loadSettings();

    // load all the mode preferences if they have some
    for(int which = Morse::TM_FIRST; which <= Morse::TM_LAST ; which++) {
        MorseMode *mode = m_morse->getMode(Morse::TrainingMode(which));
        mode->acceptPrefs();
    }

    accept();
}

void Prefs::cancel()
{
    // load all the mode preferences if they have some
    for(int which = Morse::TM_FIRST; which <= Morse::TM_LAST ; which++) {
        MorseMode *mode = m_morse->getMode(Morse::TrainingMode(which));
        mode->rejectPrefs();
    }

    if (m_oldTone != -1) {
        m_morse->setTone(m_oldTone);
        m_morse->setWPMGoal(m_oldRate);
        m_morse->createTones(m_oldRate);
    }

    reject();
}

void Prefs::testTone() {
    if (m_oldTone == -1) {
        m_oldTone = m_morse->tone();
        m_oldRate = m_morse->currentWPMGoal();
    }
    m_morse->setTone(m_tone->value());
    m_morse->createTones(m_WPMRate->value());
    m_morse->playIt('.');
}

Prefs::~Prefs()
{
}
