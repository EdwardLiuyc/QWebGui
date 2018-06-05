#include "msgbox.h"
#include <QTimerEvent>

#define ANIMATION_DURATION_MS   1000

MsgBox::MsgBox(QWidget *parent) : QWidget(parent)
  , animation_duration_( ANIMATION_DURATION_MS )
{
    text_browser_ = new QTextBrowser( this );
    text_browser_->setFocusPolicy(Qt::NoFocus);
//    text_browser_->setFont( SYSTEM_UI_FONT_10_BOLD );
    text_browser_->setAlignment( Qt::AlignCenter );
    text_browser_->setFrameShape(QFrame::NoFrame);

    opacity_ = new QGraphicsOpacityEffect( this );
    setGraphicsEffect(opacity_);

    animation_ = new QPropertyAnimation(opacity_, "opacity", this);
    animation_->setStartValue(1);
    animation_->setEndValue(0);
    animation_->setDuration(animation_duration_);
    animation_->setEasingCurve(QEasingCurve::Linear);

    this->hide();

    connect(animation_, SIGNAL(finished()), this, SLOT(close()));
}

void MsgBox::resizeEvent(QResizeEvent *event)
{
    text_browser_->setGeometry( 0, 0, width(), height() );

    QWidget::resizeEvent( event );
}

void MsgBox::setMessage(DisplayMessage &msg)
{
    QString display_str = "<h3 style=\"text-align:center;\"><br>";
    switch( msg.level_ )
    {
    case MsgLevel::kNormalMsg:
        display_str += "<font color=\"#FFFFFF\">";
        break;
    case MsgLevel::kWarningMsg:
        display_str += "<font color=\"#FFFF00\">";
        break;
    case MsgLevel::kErrorMsg:
        display_str += "<font color=\"#FF0000\">";
        break;
    default:
        display_str += "<font color=\"#FFFFFF\">";
        break;
    }

    display_str += msg.msg_;
    display_str += "</font></h3>";
    text_browser_->setText(display_str);
    if( timer_for_hiding_ == 0 )
    {
        timer_for_hiding_ = startTimer( 100 );
    }
    if( animation_->state() == QAbstractAnimation::Running )
    {
        animation_->stop();
    }

    remaining_time_ms_ = MAX_SHOWING_TIME;
    opacity_->setOpacity(1);
    this->show();
}

void MsgBox::timerEvent(QTimerEvent *event)
{
    if( timer_for_hiding_!= 0 &&  timer_for_hiding_ == event->timerId() )
    {
        remaining_time_ms_ -= 100;
        if( remaining_time_ms_ <= 0 )
        {
            killTimer( timer_for_hiding_ );
            remaining_time_ms_ = 0;
            timer_for_hiding_ = 0;
            animation_->start();
        }
    }

    QWidget::timerEvent(event);
}
