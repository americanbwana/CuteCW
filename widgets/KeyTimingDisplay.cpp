#include "KeyTimingDisplay.h"

#include <QPainter>
#include <QPolygon>
#include <qdebug.h>

KeyTimingDisplay::KeyTimingDisplay(QWidget *parent) :
    QWidget(parent)
{
}

void KeyTimingDisplay::paintEvent(QPaintEvent *event)
{
    (void)event;
    QPainter painter(this);
    painter.save();

    int minx = 100;
    //int miny = 0;
    int maxx = width() - 10;
    //int maxy = height();
    int widgetWidth = maxx - minx, widgetHeight = height();
    int lineHeight = 15, spacingHeight = widgetHeight - lineHeight * 4;

    painter.setBrush(Qt::black);
    painter.drawRect(0, 0, width(), height());

    painter.setPen(Qt::yellow);
    painter.drawText(QRect(0, lineHeight, 100, lineHeight), tr("keyed:"));
    painter.drawText(QRect(0, lineHeight*2 + spacingHeight, 100, lineHeight), tr("expected:"));

    if (m_keyedTimings.length() > 0) {
        int biggestTime = qMax(m_keyedTimings.last(), m_requiredTimings.last());

        QList<int>::const_iterator requiredSpot = m_requiredTimings.begin();
        QList<int>::const_iterator requiredEnd = m_requiredTimings.end();

        QList<int>::const_iterator keyedSpot = m_keyedTimings.begin();
        //QList<int>::const_iterator keyedEnd = m_keyedTimings.end();

        while(requiredSpot != requiredEnd) {
            QList<int>::const_iterator startingSpot = requiredSpot;
            QList<int>::const_iterator keyedStartingSpot = keyedSpot;

            painter.setBrush(Qt::yellow);
            painter.setPen(Qt::darkYellow);

            // draw the box that was actually keyed
            keyedSpot++;
            painter.drawRect(minx + (widgetWidth * *keyedStartingSpot)/biggestTime,          lineHeight,
                             (widgetWidth * (*keyedSpot - *keyedStartingSpot))/biggestTime,  lineHeight);

            // draw the required timing box that is the perfect keying
            requiredSpot++;
            painter.drawRect(minx + (widgetWidth * *startingSpot)/biggestTime,    lineHeight * 2 + spacingHeight,
                             (widgetWidth * (*requiredSpot - *startingSpot))/biggestTime, lineHeight);

            // draw a red line from the ends of the boxes to each other
            painter.setPen(Qt::darkRed);
            painter.setBrush(Qt::darkRed);
            QPolygon polygone;
            polygone << QPoint(minx + (widgetWidth * *startingSpot)/biggestTime,      lineHeight * 2 + spacingHeight)
                     << QPoint(minx + (widgetWidth * *requiredSpot)/biggestTime,      lineHeight * 2 + spacingHeight)
                     << QPoint(minx + (widgetWidth * *keyedSpot)/biggestTime,         lineHeight * 2)
                     << QPoint(minx + (widgetWidth * *keyedStartingSpot)/biggestTime, lineHeight * 2);
            painter.drawPolygon(polygone);

            //painter.drawLine(minx + (widgetWidth * *startingSpot)/biggestTime,      lineHeight*2,
            //                 minx + (widgetWidth * *keyedStartingSpot)/biggestTime, lineHeight*3);

            //painter.drawLine(minx + (widgetWidth * *spot)/biggestTime,      lineHeight*2,
            //                 minx + (widgetWidth * *keyedSpot)/biggestTime, lineHeight*3);


            requiredSpot++;
            keyedSpot++;
        }
    }

    painter.restore();
}

void KeyTimingDisplay::setTimings(QList<int> keyedTimings, QList<int> requiredTimings)
{
    m_keyedTimings = keyedTimings;
    m_requiredTimings = requiredTimings;
    update();
}

void KeyTimingDisplay::clear()
{
    m_keyedTimings.clear();
    m_requiredTimings.clear();
    update();
}

void KeyTimingDisplay::mousePressEvent(QMouseEvent *)
{
    parentWidget()->setFocus();
}

