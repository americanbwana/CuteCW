#ifndef KEYTIMINGDISPLAY_H
#define KEYTIMINGDISPLAY_H

#include <QWidget>

class KeyTimingDisplay : public QWidget
{
    Q_OBJECT
public:
    explicit KeyTimingDisplay(QWidget *parent = 0);
    void paintEvent(QPaintEvent *);

signals:

public slots:

};

#endif // KEYTIMINGDISPLAY_H
