#include "robotstatusview.h"
#include <QTimerEvent>

RobotStatusView::RobotStatusView(QWidget *parent)
    : QWidget(parent)
    , current_robot_( NULL )
    , timer_to_update_status_( 0 )
{
    opacity_ = new QGraphicsOpacityEffect( this );
    setGraphicsEffect(opacity_);

    animation_ = new QPropertyAnimation(opacity_, "opacity", this);
    animation_->setStartValue( 0 );
    animation_->setEndValue( 1 );
    animation_->setDuration( 500 );
    animation_->setEasingCurve(QEasingCurve::Linear);

    QString submodule_strs[kSubmoduleCount] = { "Over All", "GPS"  };
    for( int i = 0; i < kSubmoduleCount; ++i )
    {
        submodule_boxes_[i] = new QGroupBox( this );
        submodule_boxes_[i]->setTitle( submodule_strs[i] );
        submodule_layouts_[i] = new QGridLayout( submodule_boxes_[i] );
        submodule_layouts_[i]->setContentsMargins( 0, 0, 28, 0);
        submodule_boxes_[i]->setLayout( submodule_layouts_[i] );
        submodule_boxes_[i]->setFont( SYSTEM_UI_FONT_10_BOLD );
    }

    this->setFont( SYSTEM_UI_FONT_8_BOLD );

    QString overall_strs[OverAllLabel::kOverallLabelCount] =
    {
        "x", "0.000",
        "y", "0.000",
        "yaw", "0.000",
        "battery", "100%",
        "error", "0"
    };
    for( int i = 0; i < OverAllLabel::kOverallLabelCount; ++i )
    {
        overall_lbls_[i] = new QLabel( this );
        overall_lbls_[i]->setText( overall_strs[i] );
        overall_lbls_[i]->setFocusPolicy(Qt::NoFocus);

        if( i%2 == 0 )
        {
            overall_lbls_[i]->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
            submodule_layouts_[kOverall]->addWidget( overall_lbls_[i], i/2, i%2, 1, 3);

        }
        else
        {
            overall_lbls_[i]->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
            submodule_layouts_[kOverall]->addWidget( overall_lbls_[i], i/2, i%2 + 2, 1, 4);
        }

    }

    QString gps_strs[GPSLabel::kGpsLabelCount] =
    {
        "x", "0.000",
        "y", "0.000",
        "z", "0.000",
        "mode", "0"
    };
    for( int i = 0; i < GPSLabel::kGpsLabelCount; ++i )
    {
        gps_lbls_[i] = new QLabel( this );
        gps_lbls_[i]->setText( gps_strs[i] );
        gps_lbls_[i]->setFocusPolicy(Qt::NoFocus);

        if( i%2 == 0 )
        {
            gps_lbls_[i]->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
            submodule_layouts_[kGps]->addWidget( gps_lbls_[i], i/2, i%2, 1, 3);
        }
        else
        {
            gps_lbls_[i]->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
            submodule_layouts_[kGps]->addWidget( gps_lbls_[i], i/2, i%2+2, 1, 4);
        }
    }

    timer_to_update_status_ = startTimer( 200 );
}


void RobotStatusView::resizeEvent(QResizeEvent* event)
{
    int32_t view_wth = this->width();
    int32_t view_hgt = this->height();

    int32_t gap_wth = 10;
    int32_t gap_hgt = 5;
    int32_t box_wth = view_wth - gap_wth * 2;
    int32_t overall_row_count = submodule_layouts_[kOverall]->rowCount();
    int32_t gps_row_count = submodule_layouts_[kGps]->rowCount();
    int32_t box_hgt_overall = ( view_hgt - gap_hgt ) * overall_row_count / (overall_row_count+gps_row_count);
    int32_t lbl_hgt = 40;
    if( box_hgt_overall > lbl_hgt * overall_row_count )
        box_hgt_overall = lbl_hgt * overall_row_count;
    int32_t box_hgt_gps = ( view_hgt - gap_hgt ) * gps_row_count / (overall_row_count+gps_row_count);
    if( box_hgt_gps > 160 )
        box_hgt_gps = 160;
    submodule_boxes_[SubModules::kOverall]->setGeometry( gap_wth, 0, box_wth, box_hgt_overall);
    submodule_boxes_[SubModules::kGps]->setGeometry( gap_wth, gap_hgt + box_hgt_overall, box_wth, box_hgt_gps );

    QWidget::resizeEvent( event );
}

void RobotStatusView::timerEvent(QTimerEvent *event)
{
    if( event->timerId() == timer_to_update_status_ && current_robot_)
    {
        updateStatus();
    }

    QWidget::timerEvent( event );
}

void RobotStatusView::showEvent(QShowEvent *event)
{
    animation_->start();

    QWidget::showEvent( event );
}

void RobotStatusView::hideEvent(QHideEvent *event)
{
//    animation_->setStartValue( 1 );
//    animation_->setEndValue( 0 );
//    animation_->setDuration( 1000 );
//    animation_->setEasingCurve(QEasingCurve::Linear);
//    animation_->start();

    QWidget::hideEvent( event );
}

void RobotStatusView::setCurrentRobot(Robot *robot)
{
    current_robot_ = robot;
    if( current_robot_ == NULL )
    {
        hide();
        updateStatus();
    }
    else
        show();
}

void RobotStatusView::updateStatus()
{
    if( current_robot_ && current_robot_->connected_ )
    {
        overall_lbls_[kXValue]->setText( QString::number(current_robot_->state_.x, 'd', 3 ) );
        overall_lbls_[kYValue]->setText( QString::number(current_robot_->state_.y, 'd', 3 ) );
        overall_lbls_[KYawValue]->setText( QString::number(current_robot_->state_.yaw, 'd', 3 ) );

        overall_lbls_[KBatteryValue]->setText( QString::number(current_robot_->state_.battery * 100., 'd', 1 ) + QString("%"));
        overall_lbls_[kErrValue]->setText( QString::number( current_robot_->state_.error ) );
    }
    else
    {
        overall_lbls_[kXValue]->setText( "0.000" );
        overall_lbls_[kYValue]->setText( "0.000" );
        overall_lbls_[KYawValue]->setText( "0.000" );

        overall_lbls_[KBatteryValue]->setText( "100%" );
        overall_lbls_[kErrValue]->setText( "0" );
    }
}
