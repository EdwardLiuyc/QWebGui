#include "statusmonitorview.h"
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QMessageBox>
#include <QDebug>
#include "pugixml/pugixml.hpp"


#define SWITCH_TO_MONITOR       QString( "Switch To\nMonitor" )
#define SWITCH_TO_PATHMANAGER   QString( "Switch To\nPathManage" )
StatusMonitorView::StatusMonitorView(std::list<Robot> *robots, std::list<MapSetting> *maps, QWidget *parent, MonitorMode mode)
    : QWidget( parent )
    , robots_( robots )
    , maps_( maps )
    , current_selected_map_( NULL )
    , current_selected_robot_( NULL )
    , monitor_mode_( mode )
    , path_manage_mode_( kOldMode )
    , has_map_( false )
    , got_first_origin_( false )
    , factor_( 1.0 )
    , min_factor_( 0.5 )
    , resolution_( 0.05 )
    , origin_()
    , origin_offset_()
    , timer_update_robots_( startTimer(500) )
    , timer_manual_operating_( startTimer(500) )
    , have_manual_stop_( true )
{
    QString operations_str[kOperationCount] = {"Robots", "Maps", "Manual", "Halt", "Add In Robot", "Modify Map"};
    for( int i = 0; i < kOperationCount; ++i )
    {
        operation_btns_[i] = new QPushButton(this);
        operation_btns_[i]->setText( operations_str[i] );
        operation_btns_[i]->setFont( SYSTEM_UI_FONT_10_BOLD );
        operation_btns_[i]->setFocusPolicy( Qt::NoFocus );
        operation_btns_[i]->setCheckable( true );
    }
    operation_btns_[kModifyMap]->setEnabled( has_map_ );
    QObject::connect( operation_btns_[kSelectRobot], SIGNAL(toggled(bool)), this, SLOT(slotOnSelectRobotBtnClicked(bool)));
    QObject::connect( operation_btns_[kSelectMap], SIGNAL(toggled(bool)), this, SLOT(slotOnSelectMapBtnClicked(bool)));
    QObject::connect( operation_btns_[kChangeRunningMode], SIGNAL(toggled(bool)), this, SLOT(slotOnChangeRunningModeClicked(bool)));
    QObject::connect( operation_btns_[kRunOrHalt], SIGNAL(toggled(bool)), this, SLOT(slotOnRunOrHaltBtnClicked(bool)));
    QObject::connect( operation_btns_[kChangeAddPointMode], SIGNAL(toggled(bool)), this, SLOT(slotOnChangeAddPointModeClicked(bool)));
    QObject::connect( operation_btns_[kModifyMap], SIGNAL(toggled(bool)), this, SLOT(slotOnModifyMapClicked(bool)));

    QString image_paths[kPathMngOperationCount] =
    {
        ":/images/icons/confirm.png",
        ":/images/icons/reverse.png",
        ":/images/icons/loop.png"
    };
    for( int i = 0; i < kPathMngOperationCount; ++i )
    {
        path_mng_btns_[i] = new QPushButton(this);
        path_mng_btns_[i]->setFocusPolicy( Qt::NoFocus );
        path_mng_btns_[i]->setIcon( QIcon(image_paths[i]) );

        // TODO hover picture
        QString style = "border: none;";
        path_mng_btns_[i]->setStyleSheet(style);
    }
    QObject::connect( path_mng_btns_[kAddPoint], SIGNAL(clicked()), this, SLOT(slotOnAddPoint()));
    QObject::connect( path_mng_btns_[kSetReverseMode], SIGNAL(clicked()), this, SLOT(slotOnSetReverseMode()));
    QObject::connect( path_mng_btns_[kSetLoopMode], SIGNAL(clicked()), this, SLOT(slotOnSetLoopMode()));

    robot_select_view_ = new RobotSelectView( robots_, this );
    robot_select_view_->setVisible( false );
    robot_select_view_->setFocusPolicy( Qt::NoFocus );

    // ** return button **
    return_btn_ = new QPushButton(this);
    return_btn_->setText("back");
    return_btn_->setFocusPolicy( Qt::NoFocus );
    return_btn_->setFont( SYSTEM_UI_FONT_14_BOLD );
    QObject::connect( return_btn_, SIGNAL(clicked()), this, SLOT(slotOnReturnBtnClicked()));

    // ** switch button **
    switch_btn_ = new QPushButton( this );
    switch_btn_->setFocusPolicy( Qt::NoFocus );
    if( monitor_mode_ == MonitorMode::kStatusMonitorMode )
        switch_btn_->setText(SWITCH_TO_PATHMANAGER);
    else
        switch_btn_->setText(SWITCH_TO_MONITOR);
    switch_btn_->setFont( SYSTEM_UI_FONT_14_BOLD );
    QObject::connect( switch_btn_, SIGNAL(clicked()), this, SLOT(slotOnSwitchBtnClicked()));

    map_select_box_ = new QComboBox( this );
    map_select_box_->setFocusPolicy( Qt::NoFocus );
    map_select_box_->setVisible( false );
    map_select_box_->setFont( SYSTEM_UI_FONT_10_BOLD );
    QObject::connect( map_select_box_, SIGNAL(currentIndexChanged(int)), this, SLOT(slotOnMapSelected(int)));

    robot_select_box_ = new QComboBox( this );
    robot_select_box_->setFocusPolicy( Qt::NoFocus );
    robot_select_box_->setVisible( false );
    robot_select_box_->setFont( SYSTEM_UI_FONT_10_BOLD );
    QObject::connect( robot_select_box_, SIGNAL(currentIndexChanged(int)), this, SLOT(slotOnRobotSelected(int)));

    for( int i = 0; i < DirKey::kKeyCount; ++i )
        key_pressed_[i] = false;

    msg_box_ = new MsgBox( this );
    msg_box_->setFocusPolicy( Qt::NoFocus );

    QString modify_map_op_str[kModifyMapOpCount] = { "start", "finish", "delete", "save"};
    for( int i = 0; i < kModifyMapOpCount; ++i )
    {
        modify_map_sub_btns_[i] = new QPushButton( this );
        modify_map_sub_btns_[i]->setText( modify_map_op_str[i] );
        modify_map_sub_btns_[i]->setFont( SYSTEM_UI_FONT_10_BOLD );
        modify_map_sub_btns_[i]->setFocusPolicy( Qt::NoFocus );
        modify_map_sub_btns_[i]->setCheckable( true );
        modify_map_sub_btns_[i]->setVisible( false );

        QObject::connect( modify_map_sub_btns_[i], SIGNAL(clicked()), this, SLOT(slotHandleModifyMapBtns()) );
    }

    this->setFocus();
}

StatusMonitorView::~StatusMonitorView()
{
    if( timer_update_robots_ != 0 )
    {
        killTimer(timer_update_robots_);
        timer_update_robots_ = 0;
    }
    if( timer_manual_operating_ != 0 )
    {
        killTimer( timer_manual_operating_ );
        timer_manual_operating_ = 0;
    }
}

void StatusMonitorView::resizeEvent(QResizeEvent *event)
{
    int32_t view_wdt = this->width();
    int32_t view_hgt = this->height();

    if( !has_map_ )
    {
        origin_.setX( view_wdt*0.5 );
        origin_.setY( view_hgt*0.5 );
    }

    int32_t btn_wdt = 140;
    int32_t btn_hgt = 40;
    int32_t gap_wdt = 20;
    int32_t gap_hgt = 10;
    const int top_hgt = 30;
    for( int i = 0; i < kOperationCount; ++i )
        operation_btns_[i]->setGeometry( gap_wdt, i*(btn_hgt+gap_hgt) + top_hgt, btn_wdt, btn_hgt );

    int32_t sub_btn_wth = 100;
    int32_t sub_btn_hgt = 25;
    int32_t sub_gap_wdt = 10;
    int32_t sub_gap_hgt = 8;
    for( int i = 0; i < kModifyMapOpCount; ++i )
        modify_map_sub_btns_[i]->setGeometry( gap_wdt + btn_wdt + sub_gap_wdt
                                              , operation_btns_[kModifyMap]->geometry().top() + i * (sub_btn_hgt+sub_gap_hgt)
                                              , sub_btn_wth
                                              , sub_btn_hgt);


    int32_t switch_btn_hgt = btn_hgt * 2;
    return_btn_->setGeometry( gap_wdt, view_hgt - btn_hgt - top_hgt, btn_wdt, btn_hgt );
    switch_btn_->setGeometry( gap_wdt, view_hgt - btn_hgt - switch_btn_hgt - gap_hgt - top_hgt, btn_wdt, switch_btn_hgt );

    int32_t table_left = btn_wdt + gap_wdt * 2;
    int32_t table_wdt = ( view_wdt * 0.35 ) ;
    table_wdt = table_wdt > 600 ? 600 : table_wdt;
    robot_select_view_->setGeometry( table_left, top_hgt, table_wdt, view_hgt - top_hgt * 2);

    map_select_box_->setGeometry( table_left, (btn_hgt+gap_hgt) + top_hgt
                                  , btn_wdt, btn_hgt );
    robot_select_box_->setGeometry( table_left, top_hgt
                                  , btn_wdt, btn_hgt );

    // path manage btns
    int32_t path_mng_btn_wth = 50;
    int32_t path_mng_btn_hgt = path_mng_btn_wth;
    int32_t path_mng_btn_gap = 10;
    int32_t path_mng_btn_left = view_wdt - gap_wdt - path_mng_btn_wth;
    int32_t path_mng_btn_top = ( view_hgt - path_mng_btn_hgt * kPathMngOperationCount
            - path_mng_btn_gap * (path_mng_btn_gap - 1 ) ) / 2;
    for( int i = 0; i < kPathMngOperationCount; ++i )
    {
        path_mng_btns_[i]->setGeometry( path_mng_btn_left
                                        , path_mng_btn_top + i * (path_mng_btn_hgt+path_mng_btn_gap)
                                        , path_mng_btn_wth, path_mng_btn_hgt);
        path_mng_btns_[i]->setIconSize( path_mng_btns_[i]->size() );
    }

    // message box
    int32_t msg_box_wth = 400;
    int32_t msg_box_hgt = 80;
    int32_t msg_box_left = view_wdt - gap_wdt - msg_box_wth;
    msg_box_->setGeometry( msg_box_left, top_hgt, msg_box_wth, msg_box_hgt );

    QWidget::resizeEvent( event );
}

void StatusMonitorView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
        start_pos_for_move_ = event->pos();
    else if( event->button() == Qt::LeftButton )
    {
        if( add_point_mode_ == kInUI && has_map_ )
        {
            set_point_in_ui_ = event->pos();
            update();
        }
    }

    QWidget::mousePressEvent(event);
}

void StatusMonitorView::mouseMoveEvent(QMouseEvent *event)
{
    if( event->buttons() & Qt::RightButton )
    {
        origin_offset_single_move_ =  event->pos() - start_pos_for_move_;
        update();
    }

    QWidget::mouseMoveEvent(event);
}

void StatusMonitorView::mouseReleaseEvent(QMouseEvent *event)
{
    if( event->button() == Qt::RightButton )
    {
        origin_offset_ += origin_offset_single_move_;
        origin_offset_single_move_.setX( 0 );
        origin_offset_single_move_.setY( 0 );

    }
    else if( event->button() == Qt::LeftButton )
    {
        if( modify_map_state_ == kAddingPoints
            /*&& current_start_pos_for_marquee_!=current_end_pos_for_marquee_
            && */)
        {

            QPointF current_pos =  event->pos();
            current_adding_points_.append( current_pos );
            update();
        }
        qDebug() << end_pos_for_marquee_.size();
    }

    QWidget::mouseReleaseEvent( event );
}

void StatusMonitorView::paintEvent(QPaintEvent *event)
{
    QPainter painter( this );
    painter.setRenderHint(QPainter::Antialiasing, true);
    QPoint tmp_origin;
    if( !has_map_ )
    {
        tmp_origin = origin_ + origin_offset_ + origin_offset_single_move_;
    }
    else
    {
        // paint background
        QColor bg_color( image_.pixel( 0, 0 ) );
        painter.setBrush(bg_color);
        painter.setPen(Qt::NoPen);
        painter.drawRect(rect());

        // has map file, show map image and all robotics run in the map
        QPixmap pixmap = QPixmap::fromImage( image_ );
        int32_t view_wth = this->width();
        int32_t view_hgt = this->height();
        // scale the image
        QSize image_size = image_.size();

        int32_t scaled_image_wth = image_size.width() * factor_;
        int32_t scaled_image_hgt = image_size.height() * factor_;
        int32_t scaled_image_left = ( view_wth - scaled_image_wth ) / 2;
        int32_t scaled_image_top = ( view_hgt - scaled_image_hgt ) / 2;

        QPoint tmp_offset = origin_offset_ + origin_offset_single_move_;
        QPoint image_left_top( scaled_image_left + tmp_offset.x(), scaled_image_top + tmp_offset.y() );
        QRect target( image_left_top.x(), image_left_top.y(), scaled_image_wth, scaled_image_hgt );
        // draw the image
        painter.drawPixmap(target, pixmap, image_.rect());

        // dram the origin
        Vector2i tmp_origin_offset = map_image_info_.origin_ * factor_;
        origin_ = image_left_top + tmp_origin_offset;
        tmp_origin = origin_;

        // resolution
        resolution_ = map_image_info_.resolution_;

        if( add_point_mode_ == kInUI )
            PaintADot( &painter, set_point_in_ui_ );
    }

    paintACoordSystem( &painter, tmp_origin );

    QPen pen( Qt::blue, 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin );
    painter.setPen(pen);
    if( !robots_->empty() )
    {
        for( Robot& robot: *robots_ )
            if( robot.connected_ )
            {
                QPointF robot_pos_real( robot.state_.x, robot.state_.y );
                PaintARobot( &painter
                             , CalculateScreenPos( robot_pos_real, resolution_ , tmp_origin, factor_ )
                             , robot.state_.yaw
                             , factor_ );
            }
    }

    if( current_selected_robot_ )
        PaintRunningHistory( &painter, *(current_selected_robot_->getHistrory()), 0.2, resolution_, factor_, tmp_origin );

    painter.setBrush( QBrush(QColor(200,200,200,128)));
    painter.setPen( QPen(Qt::black, 1, Qt::DashDotLine, Qt::RoundCap, Qt::RoundJoin) );
    PaintASelectedArea( &painter, current_adding_points_ );

//    if( start_pos_for_marquee_.size() == end_pos_for_marquee_.size()
//            && start_pos_for_marquee_.size() != 0 )
//    {
//        for( int i = 0; i < start_pos_for_marquee_.size(); ++i )
//        {
//            QLinearGradient linear(start_pos_for_marquee_[i], start_pos_for_marquee_[i]+QPointF(10,10));
//            linear.setColorAt( 0, Qt::black);
//            linear.setColorAt( 1, QColor( 80, 80, 80 ));

//            linear.setSpread( QGradient::RepeatSpread );
//            painter.setBrush(linear);
//            painter.setPen( QPen(Qt::black, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin) );

//            PaintASelectedArea( &painter, start_pos_for_marquee_[i], end_pos_for_marquee_[i] );
//        }
//    }

    QWidget::paintEvent( event );
}

void StatusMonitorView::slotHandleModifyMapBtns()
{
    QPushButton* btn = qobject_cast<QPushButton *>(QObject::sender());
    if( btn == modify_map_sub_btns_[kStartSelectPoints] )
    {
        if( modify_map_sub_btns_[kStartSelectPoints]->isChecked() )
        {
            modify_map_state_ = ModifyMapState::kAddingPoints;
            modify_map_sub_btns_[kStartSelectPoints]->setEnabled( false );
        }

        modify_map_sub_btns_[kFinishSelectPoints]->setEnabled( true );
        modify_map_sub_btns_[kDeleteArea]->setEnabled( false );
        modify_map_sub_btns_[kSaveToMap]->setEnabled( false );
    }
    else if ( btn == modify_map_sub_btns_[kFinishSelectPoints] )
    {
        modify_map_state_ = ModifyMapState::kDoingNothing;
        modify_map_sub_btns_[kStartSelectPoints]->setEnabled( true );
        modify_map_sub_btns_[kFinishSelectPoints]->setEnabled( false );
        modify_map_sub_btns_[kDeleteArea]->setEnabled( true );
        modify_map_sub_btns_[kSaveToMap]->setEnabled( true );

        if( QMessageBox::information( this,
                                                 "Info",
                                                 "Add New obstacle?",
                                                 QMessageBox::Yes | QMessageBox::No,
                                                 QMessageBox::Yes)
                        ==  QMessageBox::Yes )
            qDebug() << "finished";
    }
    else if ( btn == modify_map_sub_btns_[kDeleteArea] )
    {

    }
    else if( btn == modify_map_sub_btns_[kSaveToMap] )
    {

    }
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
    else if( timer_manual_operating_ != 0 && timer_manual_operating_ == event->timerId() )
    {
        if( current_selected_robot_ == NULL )
        {
            QWidget::timerEvent( event );
            return;
        }

        bool need_add_strength = false;
        for( int i = 0; i < DirKey::kKeyCount; ++i )
        {
            if( key_pressed_[i] )
            {
                need_add_strength = true;
                break;
            }
        }

        // avoid to press up&down | left&right
        if( ( key_pressed_[kUp] && key_pressed_[kDown] )
                || ( key_pressed_[DirKey::kLeft] && key_pressed_[DirKey::kRight])
                || !need_add_strength )
        {
            manual_strength_ = 0.;
            manual_angle_ = 1.57;
            manual_vec_ = Vector2F( 0., 0. );
        }
        else
        {
            const double delta = 0.25;
            // grow a vector!
            Vector2F delta_vec[kKeyCount] =
            {
                Vector2F( 0, delta ),
                Vector2F( 0, -delta),
                Vector2F( -delta, 0 ),
                Vector2F( delta, 0)
            };

            // forward
            if( key_pressed_[kUp] )
            {
                if( manual_vec_.y() < 0. )
                    manual_vec_.setY( delta );
                else if( manual_vec_.y() >= 1.)
                    manual_vec_.setY( 1. );
                else
                    manual_vec_ += delta_vec[kUp];
            }
            else if( key_pressed_[kDown] )
            {
                if( manual_vec_.y() > 0. )
                    manual_vec_.setY( -delta );
                else if( manual_vec_.y() <= -1.)
                    manual_vec_.setY( -1. );
                else
                    manual_vec_ += delta_vec[kDown];
            }
            else
                manual_vec_.setY( 0. );

            // turning
            if( key_pressed_[kLeft] )
            {
                if( manual_vec_.x() > 0. )
                    manual_vec_.setX( -delta );
                else
                    manual_vec_ += delta_vec[kLeft];
            }
            else if( key_pressed_[kRight] )
            {
                if( manual_vec_.x() < 0. )
                    manual_vec_.setX( delta );
                else
                    manual_vec_ += delta_vec[kRight];
            }
            else
                manual_vec_.setX( 0. );
        }

        manual_strength_ = manual_vec_.manhattanLength();
        if( manual_strength_ > 1. )
            manual_strength_ = 1.;
        manual_angle_ = atan2( manual_vec_.y(), manual_vec_.x() );


        if( !DOUBLE_EQUAL( manual_strength_, 0. ) )
        {
            have_manual_stop_ = false;
            current_selected_robot_->sendCommand_ManualRun( manual_strength_, manual_angle_ );
        }
        else if( !have_manual_stop_ )
        {
            current_selected_robot_->sendCommand_ManualRun( 0., 0. );
            have_manual_stop_ = true;
        }

//        qDebug() << manual_strength_ << " " << manual_angle_ ;
    }
    QWidget::timerEvent( event );
}

void StatusMonitorView::wheelEvent(QWheelEvent *event)
{
    double last_factor = factor_;
    double delta = ( factor_ > 2. ? 0.2 : 0.1 );
    if(event->delta() > 0)
        factor_ += delta;
    else
    {
        if( factor_ <= min_factor_ )
        {
            QWidget::wheelEvent( event );
            return;
        }
        factor_ -= delta;
    }

    if( factor_ < min_factor_ )
        factor_ = min_factor_;

    QPoint tmp_origin = origin_ + origin_offset_ + origin_offset_single_move_;
    Vector2i tmp_vec = ( tmp_origin - event->pos() ) / last_factor * factor_;
    origin_offset_ = tmp_vec + event->pos() - origin_ - origin_offset_single_move_;

    QWidget::wheelEvent( event );
    update();
}

void StatusMonitorView::keyPressEvent(QKeyEvent *event)
{
    if( robot_running_mode_ == RobotRunningMode::kManual )
    {
        switch( event->key() )
        {
        case Qt::Key_Up:
            key_pressed_[DirKey::kUp] = true;
            break;
        case Qt::Key_Down:
            key_pressed_[DirKey::kDown] = true;
            break;
        case Qt::Key_Left:
            key_pressed_[DirKey::kLeft] = true;
            break;
        case Qt::Key_Right:
            key_pressed_[DirKey::kRight] = true;
            break;
        default:
            break;
        }
    }

//    QWidget::keyPressEvent( event );
}

void StatusMonitorView::keyReleaseEvent(QKeyEvent *event)
{
    if( robot_running_mode_ == RobotRunningMode::kManual )
    {
        switch( event->key() )
        {
        case Qt::Key_Up:
            key_pressed_[DirKey::kUp] = false;
            break;
        case Qt::Key_Down:
            key_pressed_[DirKey::kDown] = false;
            break;
        case Qt::Key_Left:
            key_pressed_[DirKey::kLeft] = false;
            break;
        case Qt::Key_Right:
            key_pressed_[DirKey::kRight] = false;
            break;
        default:
            break;
        }
    }

    QWidget::keyReleaseEvent( event );

}

void StatusMonitorView::slotOnReturnBtnClicked()
{
    this->hide();
    emit signalReturn();
}

void StatusMonitorView::slotOnModifyMapClicked(bool checked)
{
    for( int i = 0; i < kModifyMapOpCount; ++i )
        modify_map_sub_btns_[i]->setVisible( checked );
}

void StatusMonitorView::slotOnSwitchBtnClicked()
{
    hideSwitchableWidgets();
    monitor_mode_ = ( monitor_mode_ == kStatusMonitorMode ? kPathManageMode : kStatusMonitorMode );
    if( monitor_mode_ == kStatusMonitorMode )
    {
        switch_btn_->setText( SWITCH_TO_PATHMANAGER );
        parentWidget()->setWindowTitle("Status Monitor");
    }
    else
    {
        switch_btn_->setText( SWITCH_TO_MONITOR );
        parentWidget()->setWindowTitle("Path Management");
    }
}

void StatusMonitorView::slotOnChangeRunningModeClicked(bool checked)
{
    if( monitor_mode_ == MonitorMode::kStatusMonitorMode )
    {
        if( robots_->empty() )
            return;
    }
    else
    {
        if( !current_selected_robot_ || !current_selected_robot_->connected_)
        {
            qDebug() << "not selected or not connected!";
            operation_btns_[kChangeRunningMode]->setChecked( false );
            return;
        }
        robot_running_mode_ = checked ? RobotRunningMode::kAuto : RobotRunningMode::kManual;
        current_selected_robot_->sendCommand_SetRunningMode( robot_running_mode_ );
    }

    if( checked )
        operation_btns_[kChangeRunningMode]->setText("Auto");
    else
        operation_btns_[kChangeRunningMode]->setText("Manual");
}

void StatusMonitorView::slotOnRunOrHaltBtnClicked(bool checked)
{
    if( monitor_mode_ == MonitorMode::kStatusMonitorMode )
    {
        if( robots_->empty() )
            return;
    }
    else
    {
        if( !current_selected_robot_ || !current_selected_robot_->connected_)
        {
            qDebug() << "not selected or not connected!";
            operation_btns_[kRunOrHalt]->setChecked( false );
            return;
        }

        if( checked)
            current_selected_robot_->sendCommand_Run();
        else
            current_selected_robot_->sendCommand_Halt();
    }

    if( checked )
        operation_btns_[kRunOrHalt]->setText("Running");
    else
        operation_btns_[kRunOrHalt]->setText("Halt");
}

void StatusMonitorView::slotOnSelectRobotBtnClicked(bool checked)
{
    if( monitor_mode_ == MonitorMode::kStatusMonitorMode )
    {
        robot_select_view_->setVisible( checked );
        if( checked && operation_btns_[kSelectMap]->isChecked())
            operation_btns_[kSelectMap]->setChecked( false );
    }
    else
    {
        robot_select_box_->setVisible( checked );
        if( current_selected_robot_ )   // if has selected a robot, do nothing!
            return;

        if( checked )
        {
            robot_select_box_->clear();
            QStringList robot_names;
            robot_names.append( QString("Select Robot...") );
            if( !robots_->empty() )
                for( Robot& robot : *robots_ )
                    robot_names.append( robot.settings_.name_);
            robot_select_box_->addItems( robot_names );
            robot_select_box_->setCurrentText( 0 );
        }
    }
}

void StatusMonitorView::slotOnChangeAddPointModeClicked(bool checked)
{
    if( monitor_mode_ == MonitorMode::kStatusMonitorMode )
    {
        operation_btns_[kChangeAddPointMode]->setChecked( false );
    }
    else
    {
        if( checked )
        {
            operation_btns_[kChangeAddPointMode]->setText("Add In UI");
            add_point_mode_ = kInUI;
        }
        else
        {
            operation_btns_[kChangeAddPointMode]->setText("Add In Robot");
            add_point_mode_ = kInRobot;
        }

    }
}

void StatusMonitorView::slotOnRobotSelected(int index)
{
    // if have selected, disconnect the former socket and disconnect from the slot
    if( current_selected_robot_ )
    {
        current_selected_robot_->disconnectSocket();
        QObject::disconnect( current_selected_robot_, SIGNAL(signalRobotRcvNormalMsg(DisplayMessage&)), this, SLOT(slotOnRcvCurrentRobotMsg(DisplayMessage&)));
    }

    // select no robot!
    if( index <= 0 )
    {
        current_selected_robot_ = NULL;
        return;
    }

    std::list<Robot>::iterator it = robots_->begin();
    std::advance(it, index-1);
    if(!current_selected_robot_)
        current_selected_robot_ = &(*it);
    else
    {
        if( current_selected_robot_->settings_.name_ == it->settings_.name_ )
            return;
        else
            current_selected_robot_ = &(*it);
    }

    QObject::connect( current_selected_robot_, SIGNAL(signalRobotRcvNormalMsg(DisplayMessage&)), this, SLOT(slotOnRcvCurrentRobotMsg(DisplayMessage&)));
    current_selected_robot_->connectSocket();

}

void StatusMonitorView::slotOnRcvCurrentRobotMsg(DisplayMessage &msg)
{
    qDebug() << "recieve msg :" << msg.msg_;
    msg_box_->setMessage( msg );
}

void StatusMonitorView::slotOnSelectMapBtnClicked(bool checked)
{
    if( monitor_mode_ == MonitorMode::kStatusMonitorMode && checked && operation_btns_[kSelectRobot]->isChecked() )
        operation_btns_[kSelectRobot]->setChecked( false );

    map_select_box_->setVisible( checked );
    if( current_selected_map_ )
        return;

    map_select_box_->clear();
    QStringList map_names;
    map_names.append( QString("Select Map...") );

    if( !maps_->empty() )
        for( MapSetting& setting : *maps_ )
            map_names.append( setting.name_ );

    map_select_box_->addItems( map_names );
    map_select_box_->setCurrentIndex( 0 );
}

void StatusMonitorView::hideSwitchableWidgets()
{
    for( int i = 0; i < kOperationCount; ++i )
        operation_btns_[i]->setChecked( false );
}

void StatusMonitorView::slotOnMapSelected( int index )
{
    if( index <= 0 )
    {
        current_selected_map_ = NULL;
        has_map_ = false;
        operation_btns_[kModifyMap]->setEnabled( false );
        update();
        return;
    }

    QImageReader reader;
    std::list<MapSetting>::iterator it = maps_->begin();
    std::advance( it, index - 1);
    if(!current_selected_map_)
        current_selected_map_ = &(*it);
    else
    {
        if( current_selected_map_->image_file_name_ == it->image_file_name_ )
            return;
        else
            current_selected_map_ = &(*it);
    }

    // read image file
    has_map_ = false;
    reader.setFileName( current_selected_map_->image_file_name_ );
    if( reader.canRead() )
    {
        qDebug() << "read image: " << current_selected_map_->image_file_name_;
        image_ = reader.read();
        reader.setFormat("pgm");
        QSize image_size = image_.size();
        int32_t view_wth = this->width();
        int32_t view_hgt = this->height();
        int32_t image_wth = image_size.width();
        int32_t image_hgt = image_size.height();

        double factor_x = (double)view_wth / (double)image_wth;
        double factor_y = (double)view_hgt / (double)image_hgt;
        factor_ = std::min( factor_x, factor_y );
        min_factor_ = factor_ * 0.5;
        has_map_ = true;
    }
    else
    {
        QString strError = reader.errorString();
        qDebug() << "Read Error : " << strError;
    }

    // read image info
    if( getImageInfoFromFile( map_image_info_, current_selected_map_->image_info_file_name_.toStdString().c_str() ) == 0 )
    {
        qDebug() << map_image_info_.DebugString();
    }
    else
    {
        qDebug() << "failed to load image info!";
    }
    operation_btns_[kModifyMap]->setEnabled( has_map_ );

    update();
}

int32_t StatusMonitorView::getImageInfoFromFile(MapImageInfo &info, const char *info_filename)
{
    if( !info_filename )
        return -1;

    pugi::xml_document doc;
    if( !doc.load_file( info_filename ) ) // 0代表 load 成功，没有错误
    {
        qDebug() << "Failed in load xml file :" << info_filename;
        return -1;
    }

    pugi::xml_node info_node = doc.child( "info" );
    if( info_node.empty() )
    {
        qDebug() << "Node empty!";
        return -1;
    }

    for( pugi::xml_node node = info_node.first_child();
         node;
         node = node.next_sibling() )
    {
        if( strstr(node.name(), "origin" ) )
        {
            info.origin_.setX( node.attribute("x").as_int( ));
            info.origin_.setY( node.attribute("y").as_int( ));
        }
        else if( strstr(node.name(), "resolution" ) )
        {
            info.resolution_ = node.attribute("value").as_double();
            info.unit_ = node.attribute("unit").as_string();
        }
    }

    return 0;
}

void StatusMonitorView::slotOnAddPoint()
{
    if( !current_selected_robot_ || !current_selected_robot_->connected_)
    {
        qDebug() << "not selected or not connected!";
        return;
    }
    if( add_point_mode_ == kInRobot )
        current_selected_robot_->sendCommand_AddPoint();
    else if( add_point_mode_ == kInUI )
    {
        QPointF tmp_vector = set_point_in_ui_ - origin_;
        tmp_vector *= resolution_;
        tmp_vector /= factor_;
        current_selected_robot_->sendCommand_AddPoint( tmp_vector.x(), -tmp_vector.y(), 0.);
        qDebug() << tmp_vector;
    }
}

void StatusMonitorView::slotOnSetLoopMode()
{
    if( !current_selected_robot_ || !current_selected_robot_->connected_)
    {
        qDebug() << "not selected or not connected!";
        return;
    }
    current_selected_robot_->sendCommand_SetLoopMode();
}

void StatusMonitorView::slotOnSetReverseMode()
{
    if( !current_selected_robot_ || !current_selected_robot_->connected_)
    {
        qDebug() << "not selected or not connected!";
        return;
    }
    current_selected_robot_->sendCommand_SetReverseMode();
}


