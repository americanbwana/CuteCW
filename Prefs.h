#ifndef PREFS_H
#define PREFS_H
#include "Morse.h"
#include <QDialog>
#include <QSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>

namespace Ui {
    class Prefs;
}

class Prefs : public QDialog
{
    Q_OBJECT

public:
    explicit Prefs(Morse *morse, QWidget *parent = 0);
    ~Prefs();

public slots:
    void ok();
    void cancel();
    void testTone();

private:
    Morse *m_morse;

    QSpinBox *m_WPMRate;
    QSpinBox *m_tone;

    QCheckBox *m_StartSound;
    QComboBox *m_Lang;

    QLineEdit *m_StartSoundStr;

    int m_oldTone;
    int m_oldRate;
};

#endif // PREFS_H
