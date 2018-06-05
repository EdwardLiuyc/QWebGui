#ifndef MSGBOX_H
#define MSGBOX_H

#include <QWidget>
#include <QTextBrowser>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include "common_defines.h"

#define MAX_SHOWING_TIME    2500 /*ms*/

class QGraphicsOpacityEffect;
class QPropertyAnimation;
class MsgBox : public QWidget
{
    Q_OBJECT
public:
    explicit MsgBox(QWidget *parent = 0);

    void setMessage( DisplayMessage& msg );

signals:

protected:
    void resizeEvent(QResizeEvent* event);
    void timerEvent(QTimerEvent* event);

public slots:

private:
    QTextBrowser* text_browser_;

    int32_t timer_for_hiding_ = 0;
    int32_t remaining_time_ms_ = MAX_SHOWING_TIME;

    QGraphicsOpacityEffect* opacity_;
    QPropertyAnimation*     animation_;
    int32_t                 animation_duration_;
};

#endif // MSGBOX_H
