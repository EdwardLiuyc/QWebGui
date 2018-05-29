#include "statusmonitorview.h"
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>

StatusMonitorView::StatusMonitorView(std::list<Robot> *robots, std::list<MapSetting> *maps, QWidget *parent, MonitorMode mode)
    : QWidget(parent)
    , robots_(robots)
    , maps_(maps)
    , has_map_( false )
    , got_first_origin_( false )
    , factor_( 1.0 )
    , origin_()
    , origin_offset_()
    , timer_update_robots_( startTimer(500) )
    , mode_( mode )
{
    QString operations_str[kOperationCount] = {"Robots", "Maps"};
    for( int i = 0; i < kOperationCount; ++i )
    {
        operation_btns_[i] = new QPushButton(this);
        operation_btns_[i]->setText( operations_str[i] );
        operation_btns_[i]->setFont( SYSTEM_UI_FONT_12_BOLD );
        operation_btns_[i]->setFocusPolicy( Qt::NoFocus );
        operation_btns_[i]->setCheckable( true );
    }
    QObject::connect( operation_btns_[kSelectRobot], SIGNAL(toggled(bool)), this, SLOT(slotOnSelectRobotBtnClicked(bool)));
    QObject::connect( operation_btns_[kSelectMap], SIGNAL(toggled(bool)), this, SLOT(slotOnSelectMapBtnClicked(bool)));

    robot_select_view_ = new RobotSelectView( robots_, this );
    robot_select_view_->setVisible( false );

    return_btn_ = new QPushButton(this);
    return_btn_->setText("back");
    return_btn_->setFocusPolicy( Qt::NoFocus );
    QPalette pal = return_btn_->palette();
    pal.setColor(QPalette::ButtonText,QColor(255,0,0));
    return_btn_->setPalette(pal);
    return_btn_->setFont( SYSTEM_UI_FONT_14_BOLD );
    QObject::connect( return_btn_, SIGNAL(clicked()), this, SLOT(slotOnReturnBtnClicked()));

    map_select_box_ = new QComboBox( this );
    map_select_box_->setVisible( false );
    map_select_box_->setFont( SYSTEM_UI_FONT_10_BOLD );
}

void StatusMonitorView::resizeEvent(QResizeEvent *event)
{
    int32_t view_wdt = this->width();
    int32_t view_hgt = this->height();

    origin_.setX( view_wdt*0.5 );
    origin_.setY( view_hgt*0.5 );

    int32_t btn_wdt = 140;
    int32_t btn_hgt = 40;
    int32_t gap_wdt = 20;
    int32_t gap_hgt = 10;
    const int top_hgt = 30;
    for( int i = 0; i < kOperationCount; ++i )
        operation_btns_[i]->setGeometry( gap_wdt, i*(btn_hgt+gap_hgt) + top_hgt, btn_wdt, btn_hgt );

    return_btn_->setGeometry( gap_wdt, view_hgt - btn_hgt - top_hgt, btn_wdt, btn_hgt );

    int32_t table_left = btn_wdt + gap_wdt * 2;
    int32_t table_wdt = ( view_wdt * 0.35 ) ;
    table_wdt = table_wdt > 600 ? 600 : table_wdt;
    robot_select_view_->setGeometry( table_left, top_hgt, table_wdt, view_hgt - top_hgt * 2);

    map_select_box_->setGeometry( table_left, (btn_hgt+gap_hgt) + top_hgt
                                  , btn_wdt, btn_hgt );

    QWidget::resizeEvent( event );
}

void StatusMonitorView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
        start_pos_ = event->pos();

    QWidget::mousePressEvent(event);
}

void StatusMonitorView::mouseMoveEvent(QMouseEvent *event)
{
    if( event->buttons() & Qt::LeftButton )
    {
        origin_offset_single_move_ =  event->pos() - start_pos_;
        update();
    }

    QWidget::mouseMoveEvent(event);
}

void StatusMonitorView::mouseReleaseEvent(QMouseEvent *event)
{
    origin_offset_ += origin_offset_single_move_;
    origin_offset_single_move_.setX( 0 );
    origin_offset_single_move_.setY( 0 );

    QWidget::mouseReleaseEvent( event );
}

void StatusMonitorView::paintEvent(QPaintEvent *event)
{
    QPainter painter( this );
    painter.setRenderHint(QPainter::Antialiasing, true);
    if( !has_map_ )
    {
        QPoint tmp_origin = origin_ + origin_offset_ + origin_offset_single_move_;
        paintACoordSystem( &painter, tmp_origin );

        QPen pen( Qt::white, 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin );
        painter.setPen(pen);

        if( !robots_->empty() )
        {
            for( Robot& robot: *robots_ )
                if( robot.connected_ )
                {
                    QPointF robot_pos_real( robot.state_.x, robot.state_.y );
                    PaintARobot( &painter
                                 , CalculateScreenPos( robot_pos_real, 0.05 , tmp_origin, factor_ )
                                 , robot.state_.yaw
                                 , factor_ );
                }
        }
    }
    else
    {
        // has map file, show map image and all robotics run in the map
    }

    QWidget::paintEvent( event );
}

void StatusMonitorView::timerEvent(QTimerEvent *event)
{
    if( timer_update_robots_ == event->timerId() )
    {
        if( !robots_->empty() )
        {
            for( Robot& robot: *robots_ )
                if( robot.connected_ )
                {
                    update();
                    break;
                }
        }
    }
    else
        QWidget::timerEvent( event );
}

void StatusMonitorView::wheelEvent(QWheelEvent *event)
{
    if(event->delta() > 0)
    {
        factor_ += 0.1;
    }
    else
    {
        if( factor_ >= 0.6)
        factor_ -= 0.1;
    }
    update();
}

void StatusMonitorView::slotOnReturnBtnClicked()
{
    this->hide();
    emit signalReturn();
}

void StatusMonitorView::slotOnSelectRobotBtnClicked(bool checked)
{
    if( checked && operation_btns_[kSelectMap]->isChecked())
        operation_btns_[kSelectMap]->setChecked( false );

    robot_select_view_->setVisible( checked );
}

void StatusMonitorView::slotOnSelectMapBtnClicked(bool checked)
{
    if( checked && operation_btns_[kSelectRobot]->isChecked() )
        operation_btns_[kSelectRobot]->setChecked( false );

    map_select_box_->setVisible( checked );
    map_select_box_->clear();
    QStringList map_names;
    map_names.append( QString("Select Map...") );

    if( !maps_->empty() )
        for( MapSetting& setting : *maps_ )
            map_names.append( setting.name_ );

    map_select_box_->addItems( map_names );
}
