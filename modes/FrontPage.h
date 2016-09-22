#ifndef FRONTPAGE_H
#define FRONTPAGE_H

#include "MorseMode.h"

class FrontPage : public MorseMode
{
public:
    FrontPage(Morse *parent, Ui::MainWindow *ui);

    void switchToMode();
    void setupWidgets();

    QString helpText();
    QString name();
    QString rawName();
};

#endif // FRONTPAGE_H
