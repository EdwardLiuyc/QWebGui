#include "statusmonitorview.h"
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QMessageBox>
#include <QDebug>
#include <QShortcut>
#include <algorithm>
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
    , last_target_point_set_in_ui_( 0., 0. )
    , need_restart_record_target_list_( false )
    , has_map_( false )
    , got_first_origin_( false )
    , is_managing_points_( false )
    , factor_( 1.0 )
    , min_factor_( 0.5 )
    , resolution_( 0.05 )
    , origin_()
    , origin_offset_()
    , timer_update_robots_( startTimer(500) )
    , timer_manual_operating_( startTimer(500) )
    , have_manual_stop_( true )
    , is_managing_path_( false )
    , path_manage_state_( PICKING_FIRST_POINT )
{
    QString operation_str[kOperationCount] = {"Robots", "Maps", "Manual", "Halt", "Manage Points", "Modify Map", "Manage Path"};
    QString operation_help_tip[kOperationCount] =
    {
        "Select a robot and it will be connected \n"
        "automatically!",

        "Select a map",

        "You can change the running mode between \n"
        "\"Manual\" and \"Auto\" if you have \n"
        "connected to a robot",

        "You can Halt or Restart the robot if connected",

        "Add the target points in UI or Robot \n"
        "itself.",

        "In this mode, you can add some obstacle \n"
        "area into the map, but first you should \n"
        "have loaded a map, this function is \n"
        "enabled only in Path Management Mode.",

        ""

    };
    for( int i = 0; i < kOperationCount; ++i )
    {
        operation_btns_[i] = new QPushButton(this);
        operation_btns_[i]->setText( operation_str[i] );
        operation_btns_[i]->setFont( SYSTEM_UI_FONT_10_BOLD );
        operation_btns_[i]->setFocusPolicy( Qt::NoFocus );
        operation_btns_[i]->setCheckable( true );
        operation_btns_[i]->setToolTip( operation_help_tip[i] );
    }
    operation_btns_[kModifyMap]->setEnabled( has_map_ );
    QObject::connect( operation_btns_[kSelectRobot], SIGNAL(toggled(bool)), this, SLOT(slotOnSelectRobotBtnClicked(bool)));
    QObject::connect( operation_btns_[kSelectMap], SIGNAL(toggled(bool)), this, SLOT(slotOnSelectMapBtnClicked(bool)));
    QObject::connect( operation_btns_[kChangeRunningMode], SIGNAL(toggled(bool)), this, SLOT(slotOnChangeRunningModeClicked(bool)));
    QObject::connect( operation_btns_[kRunOrHalt], SIGNAL(toggled(bool)), this, SLOT(slotOnRunOrHaltBtnClicked(bool)));
    QObject::connect( operation_btns_[kManagePoints], SIGNAL(toggled(bool)), this, SLOT(slotOnPointsManageClicked(bool)));
    QObject::connect( operation_btns_[kModifyMap], SIGNAL(toggled(bool)), this, SLOT(slotOnModifyMapClicked(bool)));
    QObject::connect( operation_btns_[kManagePath], SIGNAL(toggled(bool)), this, SLOT(slotOnManagePathClicked(bool)));

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
    QKeySequence short_cuts_[kModifyMapOpCount] =
    {
        QKeySequence(tr("Alt+S", "start")),
        QKeySequence(tr("Alt+F", "finish")),
        QKeySequence(tr("Alt+D", "delete")),
        QKeySequence(tr("Ctrl+S", "save"))
    };
    for( int i = 0; i < kModifyMapOpCount; ++i )
    {
        modify_map_sub_btns_[i] = new QPushButton( this );
        modify_map_sub_btns_[i]->setText( modify_map_op_str[i] );
        modify_map_sub_btns_[i]->setFont( SYSTEM_UI_FONT_10_BOLD );
        modify_map_sub_btns_[i]->setFocusPolicy( Qt::NoFocus );
        modify_map_sub_btns_[i]->setVisible( false );

        modify_map_sub_btns_[i]->setShortcut(short_cuts_[i]);

        QObject::connect( modify_map_sub_btns_[i], SIGNAL(clicked()), this, SLOT(slotHandleModifyMapBtns()) );
    }
    modify_map_sub_btns_[kFinishSelectPoints]->setEnabled( false );
    modify_map_sub_btns_[kDeletingArea]->setCheckable( true );

    QString points_mng_op_str[kPointsMngOperationCount] =
    {
        "add in robot", "delete point"
    };
    for( int i = 0; i < kPointsMngOperationCount; ++i )
    {
        point_mng_sub_btns_[i] = new QPushButton( this );
        point_mng_sub_btns_[i]->setText( points_mng_op_str[i] );
        point_mng_sub_btns_[i]->setFont( SYSTEM_UI_FONT_10_BOLD );
        point_mng_sub_btns_[i]->setFocusPolicy( Qt::NoFocus );
        point_mng_sub_btns_[i]->setCheckable( true );
        point_mng_sub_btns_[i]->setVisible( false );

        QObject::connect( point_mng_sub_btns_[i], SIGNAL(clicked()), this, SLOT(slotHandlePointsMngBtns()) );
    }

    QString path_mng_sub_btn_str[kPathMngSubOperationCount] =
    {
        "connect points", "pick ends", "modify a path"
    };
    for( int i = 0; i < kPathMngSubOperationCount; ++i )
    {
        path_mng_sub_btns_[i] = new QPushButton( this );
        path_mng_sub_btns_[i]->setText( path_mng_sub_btn_str[i] );
        path_mng_sub_btns_[i]->setFont( SYSTEM_UI_FONT_10_BOLD );
        path_mng_sub_btns_[i]->setFocusPolicy( Qt::NoFocus );
        path_mng_sub_btns_[i]->setCheckable( true );
        path_mng_sub_btns_[i]->setVisible( false );

        QObject::connect( path_mng_sub_btns_[i], SIGNAL(clicked()), this, SLOT(slotHandlePathMngSubBtns()) );
    }

    robot_status_view_ = new RobotStatusView( this );
    robot_status_view_->setFocusPolicy( Qt::NoFocus );
    robot_status_view_->setVisible( false );

    this->setFocus();
    this->setMouseTracking( true );
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

    int32_t sub_btn_wth = 120;
    int32_t sub_btn_hgt = 25;
    int32_t sub_gap_wdt = 10;
    int32_t sub_gap_hgt = 8;
    for( int i = 0; i < kModifyMapOpCount; ++i )
        modify_map_sub_btns_[i]->setGeometry( gap_wdt + btn_wdt + sub_gap_wdt
                                              , operation_btns_[kModifyMap]->geometry().top() + i * (sub_btn_hgt+sub_gap_hgt)
                                              , sub_btn_wth
                                              , sub_btn_hgt);
    for( int i = 0; i < kPointsMngOperationCount; ++i )
        point_mng_sub_btns_[i]->setGeometry( gap_wdt + btn_wdt + sub_gap_wdt
                                             , operation_btns_[kManagePoints]->geometry().top() + i * (sub_btn_hgt+sub_gap_hgt)
                                             , sub_btn_wth
                                             , sub_btn_hgt);
    for( int i = 0; i < kPathMngSubOperationCount; ++i )
        path_mng_sub_btns_[i]->setGeometry( gap_wdt + btn_wdt + sub_gap_wdt
                                            , operation_btns_[kManagePath]->geometry().top() + i * (sub_btn_hgt+sub_gap_hgt)
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
    int32_t path_mng_btn_left = ( view_wdt - path_mng_btn_wth * kPathMngOperationCount - path_mng_btn_gap * (kPathMngOperationCount - 1 ) ) * 0.5;
    int32_t path_mng_btn_top = ( view_hgt - top_hgt - path_mng_btn_hgt );
    for( int i = 0; i < kPathMngOperationCount; ++i )
    {
        path_mng_btns_[i]->setGeometry( path_mng_btn_left + i * (path_mng_btn_wth+path_mng_btn_gap)
                                        , path_mng_btn_top
                                        , path_mng_btn_wth, path_mng_btn_hgt);
        path_mng_btns_[i]->setIconSize( path_mng_btns_[i]->size() );
    }

    // message box
    int32_t msg_box_wth = 400;
    int32_t msg_box_hgt = 80;
    int32_t msg_box_left = ((view_wdt-msg_box_wth) >> 1);
    int32_t msg_box_top = ((view_hgt-msg_box_hgt) >> 1);
    msg_box_->setGeometry( msg_box_left, msg_box_top, msg_box_wth, msg_box_hgt );

    // status view
    int32_t status_view_wth = 230;
    int32_t status_view_hgt = view_hgt - top_hgt * 2;
    int32_t status_view_left = view_wdt - gap_wdt - status_view_wth;
    robot_status_view_->setGeometry( status_view_left, top_hgt, status_view_wth, status_view_hgt);

    QWidget::resizeEvent( event );
}

int32_t StatusMonitorView::findTheNearestPoint(QPointF &src_point, QList<PointWithInfo> &dst_points, double *dis)
{
    if( !dis || dst_points.empty() )
        return -1;

    double min_distance = 1.e10;
    int32_t index = -1;
    for( int i = 0; i < dst_points.size(); ++i )
    {
        double dist = CalculateDistance( dst_points.at(i).point, src_point );
        if( dist <= min_distance )
        {
            index = i;
            min_distance = dist;
        }
    }

    *dis = min_distance;
    return index;
}

void StatusMonitorView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
        start_pos_for_mouse_move_ = event->pos();
    else if( event->button() == Qt::LeftButton )
    {
        if( is_managing_path_ )
        {
            int32_t current_index = -1;
            for( int i = 0; i < points_maybe_selected_.size(); ++i )
            {
                if( points_maybe_selected_.at(i) )
                {
                    current_index = i;
                    break;
                }
            }

            mutex_.lock();
            switch ( path_manage_state_ )
            {
            case PICKING_FIRST_POINT:
                current_first_point_index_ = current_index;
                if( current_first_point_index_ >= 0 )
                {
                    path_manage_state_ = PICKING_CONNECTING_POINT;
                    tmp_show_line_end_ = event->pos();
                }
                break;
            case PICKING_CONNECTING_POINT:
                break;
            case PICKING_PATH_START:
                break;
            case PICKING_PATH_END:
                break;
            case PICKING_MODIFYING_PATH:
                break;
            default:
                break;
            }
            mutex_.unlock();

            update();
        }
        else if( is_managing_points_ && add_point_mode_ == kInUI && has_map_ )
        {
            QPointF current_pos = event->pos();
            QPoint tmp_origin = origin_ + origin_offset_ + origin_offset_single_move_;
            last_target_point_set_in_ui_ = CalculateRobotPos( current_pos, resolution_, tmp_origin, factor_);

            if( need_restart_record_target_list_ )
            {
                target_points_.clear();
                need_restart_record_target_list_ = false;
                points_maybe_selected_.clear();
            }
            update();
        }
    }

    QWidget::mousePressEvent(event);
}

void StatusMonitorView::mouseMoveEvent(QMouseEvent *event)
{
    QPointF current_pos = event->pos();
    if( event->buttons() & Qt::RightButton )
    {   
        origin_offset_single_move_ =  event->pos() - start_pos_for_mouse_move_;
        update();
    }
    else if ( is_modifying_map_ )
    {
        if( modify_map_state_ == kAddingPoints && !tmp_show_current_adding_points_.empty())
        {
            tmp_show_current_adding_points_[tmp_show_current_adding_points_.size()-1]
                    = CalculateRobotPos( current_pos, resolution_, origin_ + origin_offset_ + origin_offset_single_move_, factor_ );
            update();
        }
        else if( modify_map_state_ == kDeletingArea && !modifyed_points_sets_.empty() )
        {
            QPointF current_pos_in_map = CalculateRobotPos( current_pos, resolution_, origin_ + origin_offset_ + origin_offset_single_move_, factor_ );
            bool need_update = false;
            for( int i = 0; i < modifyed_points_sets_.size(); ++i )
            {
                auto& polygon = modifyed_points_sets_.at(i);
                bool inside = IsInsidePoly( current_pos_in_map, polygon );
                if( inside != selected_to_delete_[i])
                {
                    selected_to_delete_[i] = inside;
                    need_update = true;
                }
            }

            if( need_update )
                update();
        }
    }
    else if( !target_points_.empty()
             && ((is_managing_points_ && add_point_mode_ == kDelete ) || is_managing_path_) )
    {
        QPointF current_pos_in_map = CalculateRobotPos( current_pos, resolution_, origin_ + origin_offset_ + origin_offset_single_move_, factor_ );
        double min_distance = 0.;
        int32_t index = findTheNearestPoint( current_pos_in_map, target_points_, &min_distance );
        // change to distance in screen
        min_distance /= resolution_;
        min_distance *= factor_;

        bool need_update = false;
        for( int i = 0; i < points_maybe_selected_.size(); ++i )
        {
            if( i != index )
            {
                if( points_maybe_selected_.at(i) != false )
                {
                    need_update = true;
                    points_maybe_selected_[i] = false;
                }
            }
            else
            {
                if( min_distance <= 20 )
                {
                    if( points_maybe_selected_.at(i) != true )
                    {
                        need_update = true;
                        points_maybe_selected_[i] = true;
                    }
                }
                else
                {
                    if( points_maybe_selected_.at(i) != false )
                    {
                        need_update = true;
                        points_maybe_selected_[i] = false;
                    }
                }
            }
        }

        if( current_first_point_index_ >= 0 && event->buttons() & Qt::LeftButton )
        {
            tmp_show_line_end_ = current_pos;
            need_update = true;
        }

        if( need_update )
            update();
    }

    QWidget::mouseMoveEvent(event);
}

void StatusMonitorView::addConnectionToPoints()
{
    if( current_connecting_point_index_ < 0 || current_first_point_index_ < 0
            || current_connecting_point_index_ == current_first_point_index_ )
        return;

    auto& start_point = target_points_[ current_first_point_index_ ];
    auto& end_point = target_points_[ current_connecting_point_index_ ];
    if( start_point.id == end_point.id )
        return;

    if( std::find(start_point.linked_nodes.begin(), start_point.linked_nodes.end(), end_point.id ) == start_point.linked_nodes.end() )
        start_point.linked_nodes.push_back( end_point.id );
    if( std::find(end_point.linked_nodes.begin(), end_point.linked_nodes.end(), start_point.id) == end_point.linked_nodes.end() )
        end_point.linked_nodes.push_back( start_point.id );

    current_connecting_point_index_ = current_first_point_index_ = -1;
}

void StatusMonitorView::mouseReleaseEvent(QMouseEvent *event)
{
    if( event->button() == Qt::RightButton )
    {
        origin_offset_ += origin_offset_single_move_;
        origin_offset_single_move_.setX( 0 );
        origin_offset_single_move_.setY( 0 );
    }
    else if( is_modifying_map_ )
    {
        if( event->button() == Qt::LeftButton && modify_map_state_ == kAddingPoints)
        {
            QPointF current_pos = event->pos();
            QPointF relative_pos_in_map = CalculateRobotPos( current_pos, resolution_, origin_ + origin_offset_ + origin_offset_single_move_, factor_ );
            current_adding_points_.append( relative_pos_in_map );

            mutex_.lock();
            tmp_show_current_adding_points_.clear();
            tmp_show_current_adding_points_.append( current_adding_points_ );
            tmp_show_current_adding_points_.append( relative_pos_in_map );
            mutex_.unlock();
            update();
        }
        else if( modify_map_state_ == kDeletingArea && !modifyed_points_sets_.empty() )
        {
            for( int i = 0; i < selected_to_delete_.size(); ++i )
                if( selected_to_delete_.at(i) )
                {
                    modifyed_points_sets_.removeAt( i );
                    selected_to_delete_.removeAt( i );
                    break;
                }

            update();
        }
    }
    else if( is_managing_path_ && !target_points_.empty() && event->button() == Qt::LeftButton )
    {
        int current_index = -1;
        for( int i = 0; i < points_maybe_selected_.size(); ++i )
        {
            if (points_maybe_selected_.at(i))
            {
                current_index = i;
                break;
            }
        }

        mutex_.lock();
        switch ( path_manage_state_ )
        {
        case PICKING_FIRST_POINT:
            break;
        case PICKING_CONNECTING_POINT:
            current_connecting_point_index_ = current_index;
            path_manage_state_ = PICKING_FIRST_POINT;

            if( current_index >= 0 && current_index != current_first_point_index_)
                addConnectionToPoints();
            else
                tmp_show_line_end_ = CalculateScreenPos( target_points_.at(current_first_point_index_).point,
                                            resolution_, origin_ + origin_offset_ + origin_offset_single_move_, factor_ );

            break;
        case PICKING_PATH_START:
            break;
        case PICKING_PATH_END:
            break;
        case PICKING_MODIFYING_PATH:
            break;
        default:
            break;
        }
        mutex_.unlock();

        update();

    }
    else if( event->button() == Qt::LeftButton
             && is_managing_points_
             && !target_points_.empty()
             && add_point_mode_ == kDelete )
    {
        for( int i = 0; i < target_points_.size(); ++i )
            if( points_maybe_selected_.at(i) )
            {
                target_points_.removeAt( i );
                points_maybe_selected_.removeAt( i );
                break;
            }

        update();
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
        tmp_origin  = origin_ + origin_offset_ + origin_offset_single_move_;
    }
    else
    {
        // paint background
        QColor bg_color( image_.pixel( 0, 0 ) );
        painter.setBrush(bg_color);
        painter.setPen(Qt::NoPen);
        painter.drawRect(rect());

        // calculate left-top of the image
        tmp_origin = origin_ + origin_offset_ + origin_offset_single_move_;
        QPointF image_left_top = tmp_origin - map_image_info_.origin_ * factor_;

        QPixmap pixmap = QPixmap::fromImage( image_ );
        // scale the image
        QSize image_size = image_.size();
        int32_t scaled_image_wth = image_size.width() * factor_;
        int32_t scaled_image_hgt = image_size.height() * factor_;
        QRect target( image_left_top.x(), image_left_top.y(), scaled_image_wth, scaled_image_hgt );
        painter.drawPixmap( target, pixmap, image_.rect() );

        QPointF screen_point;
        if( add_point_mode_ == kInUI )
        {
            screen_point = CalculateScreenPos( last_target_point_set_in_ui_, resolution_, tmp_origin, factor_);
            PaintATargetPoint( &painter, screen_point );
        }
        for( int i = 0; i < target_points_.size(); ++i )
        {
            auto& target_point = target_points_.at(i);
            screen_point = CalculateScreenPos( target_point.point, resolution_, tmp_origin, factor_);
            QColor color;
            if( points_maybe_selected_.at(i) )
                color = QColor(Qt::blue);
            else
                color = QColor(Qt::red);

            PaintATargetPoint( &painter, screen_point, color );

            painter.setPen( QPen(QColor(Qt::red), 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin) );
            for( auto& index : target_point.linked_nodes )
            {
                if( index <= target_point.id )
                    continue;

                for( auto& tmp_point : target_points_ )
                    if( tmp_point.id == index )
                    {
                        painter.drawLine( screen_point, CalculateScreenPos( tmp_point.point, resolution_, tmp_origin, factor_) );
                        break;
                    }
            }

            if( current_first_point_index_ >= 0 && i == current_first_point_index_ )
            {
                painter.drawLine( screen_point, tmp_show_line_end_ );
            }
        }
    }

    paintACoordSystem( &painter, tmp_origin );

    if( !is_managing_path_ && !robots_->empty() )
    {
        QPen pen( Qt::blue, 1.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin );
        painter.setPen(pen);
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
    PaintASelectedMapArea( &painter, tmp_show_current_adding_points_, resolution_, tmp_origin, factor_ );

    QLinearGradient linear(QPointF(0, 0), QPointF(10,10));
    linear.setColorAt( 0, QColor( 230, 230, 230, 220));
    linear.setColorAt( 1, QColor( 120, 120, 120, 220 ));
    linear.setSpread( QGradient::ReflectSpread );
    painter.setPen( QPen(Qt::black, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin) );
    if( selected_to_delete_.size() == modifyed_points_sets_.size() )
    {
        if( !modifyed_points_sets_.empty() )
            for( int i = 0; i < modifyed_points_sets_.size(); ++i )
            {
                if( !selected_to_delete_.at(i) )
                    painter.setBrush( linear );
                else
                    painter.setBrush( QColor(255,255,0,180));
                PaintASelectedMapArea( &painter, modifyed_points_sets_[i], resolution_, tmp_origin, factor_ );
            }
    }

    QWidget::paintEvent( event );
}

void StatusMonitorView::slotHandlePointsMngBtns()
{
    QPushButton* btn = qobject_cast<QPushButton *>(QObject::sender());
    if( btn == point_mng_sub_btns_[kChangeAddPointMode] )
    {
        if( btn->isChecked() )
        {
            btn->setText("Add In UI");
            add_point_mode_ = kInUI;
        }
        else
        {
            btn->setText("Add In Robot");
            add_point_mode_ = kInRobot;
        }
    }
    else if( btn == point_mng_sub_btns_[kDeletePoint] )
    {
        if( btn->isChecked() )
        {
            point_mng_sub_btns_[kChangeAddPointMode]->setEnabled( false );
            add_point_mode_ = kDelete;
        }
        else
        {
            point_mng_sub_btns_[kChangeAddPointMode]->setEnabled( true );
            add_point_mode_ = point_mng_sub_btns_[kChangeAddPointMode]->isChecked() ? kInUI : kInRobot;
        }
    }
}

void StatusMonitorView::slotHandlePathMngSubBtns()
{
    QPushButton* btn = qobject_cast<QPushButton *>(QObject::sender());
    if( btn == path_mng_sub_btns_[kConnectPoints] )
    {

    }
}

void StatusMonitorView::slotHandleModifyMapBtns()
{
    QPushButton* btn = qobject_cast<QPushButton *>(QObject::sender());
    if( btn == modify_map_sub_btns_[kStartSelectPoints] )
    {
        modify_map_state_ = ModifyMapState::kAddingPoints;
        modify_map_sub_btns_[kStartSelectPoints]->setEnabled( false );

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

        if( !current_adding_points_.empty() )
        {
            QMessageBox msg;
            msg.setIcon( QMessageBox::Question );
            msg.setText( "Sure you want to add a new obstacle in the map?" );
            msg.setInformativeText( "You will have a static obstacle and you can delete it later!");
            msg.setStandardButtons( QMessageBox::Yes | QMessageBox::No );
            msg.setDefaultButton(QMessageBox::Yes);
            if( msg.exec() == QMessageBox::Yes )
            {
                modifyed_points_sets_.append( current_adding_points_ );
                selected_to_delete_.append( false );
                current_adding_points_.clear();
                tmp_show_current_adding_points_.clear();

                update();
            }
        }

    }
    else if ( btn == modify_map_sub_btns_[kDeleteArea] )
    {
        bool checked = btn->isChecked();
        if( checked )
        {
            modify_map_state_ = ModifyMapState::kDeletingArea;

            modify_map_sub_btns_[kStartSelectPoints]->setEnabled( false );
            modify_map_sub_btns_[kFinishSelectPoints]->setEnabled( false );
        }
        else
        {
            modify_map_state_ = ModifyMapState::kDoingNothing;

            modify_map_sub_btns_[kStartSelectPoints]->setEnabled( true );
            modify_map_sub_btns_[kFinishSelectPoints]->setEnabled( false );
        }

    }
    else if( btn == modify_map_sub_btns_[kSaveToMap] )
    {
        modify_map_state_ = ModifyMapState::kDoingNothing;
        if( modifyed_points_sets_.empty() || currentConnected() != 0 )
            return;

        for( int i = 0; i < modifyed_points_sets_.size(); ++i )
            current_selected_robot_->sendCommand_AddObstacleArea( modifyed_points_sets_.at(i));

        current_selected_robot_->sendCommand_CommandFinish("AddObstacleArea", 1.);
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

    QPoint current_pos = event->pos();
    QPoint tmp_origin = origin_ + origin_offset_ + origin_offset_single_move_;
    Vector2i tmp_vec = ( tmp_origin - current_pos ) / last_factor * factor_;
    origin_offset_ = tmp_vec + current_pos - origin_ - origin_offset_single_move_;

    update();
    QWidget::wheelEvent( event );
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
    QWidget::keyPressEvent( event );
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

    operation_btns_[kManagePath]->setEnabled( !checked );
    is_modifying_map_ = checked;
}

void StatusMonitorView::slotOnManagePathClicked(bool checked)
{
    if( target_points_.empty() )
    {
        operation_btns_[kManagePath]->setChecked( false );
        DisplayMessage msg;
        msg.level_ = kErrorMsg;

        msg.msg_ = "There is no target point!";
        msg_box_->setMessage( msg );
        return;
    }

    for( int i = 0; i < kManagePath; ++i )
    {
        if( i != kModifyMap )
            operation_btns_[i]->setEnabled( !checked );
        else
            operation_btns_[i]->setEnabled( !checked && has_map_ );
    }
    is_managing_path_ = checked;
    update();

    for( int i = 0; i < kPathMngSubOperationCount; ++i )
        path_mng_sub_btns_[i]->setVisible( checked );

    if( is_managing_path_ )
    {
        if( path_manage_nodes_.empty()
            || path_manage_nodes_.size() != target_points_.size() )
        {
            path_manage_nodes_.clear();
            for( int i = 0; i < target_points_.size(); ++i )
                path_manage_nodes_.append( NodeInPath() );
        }
    }
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
        if( currentConnected() != 0 )
        {
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
        if( currentConnected() != 0 )
        {
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

void StatusMonitorView::slotOnPointsManageClicked(bool checked)
{
    if( monitor_mode_ == MonitorMode::kStatusMonitorMode )
    {

    }
    else if( monitor_mode_ == MonitorMode::kPathManageMode )
    {
        for( int i = 0; i < kPointsMngOperationCount; ++i )
            point_mng_sub_btns_[i]->setVisible( checked );

        operation_btns_[kManagePath]->setEnabled( !checked );
        is_managing_points_ = checked;
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
        robot_status_view_->setCurrentRobot(current_selected_robot_);
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
    robot_status_view_->setCurrentRobot(current_selected_robot_);
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
    int32_t view_wth = this->width();
    int32_t view_hgt = this->height();
    QSize image_size;
    reader.setFileName( current_selected_map_->image_file_name_ );
    if( reader.canRead() )
    {
        // qDebug() << "read image: " << current_selected_map_->image_file_name_;
        // step1 calculate factor
        image_ = reader.read();
        reader.setFormat("pgm");
        image_size = image_.size();
        int32_t image_wth = image_size.width();
        int32_t image_hgt = image_size.height();

        double factor_x = (double)view_wth / (double)image_wth;
        double factor_y = (double)view_hgt / (double)image_hgt;
        factor_ = std::min( factor_x, factor_y );
        min_factor_ = factor_ * 0.5;
    }
    else
    {
        DisplayMessage msg;
        msg.level_ = kErrorMsg;
        msg.msg_ = reader.errorString();
        msg_box_->setMessage( msg );

        return;
    }

    has_map_ = true;
    // read image info
    if( getImageInfoFromFile( map_image_info_, current_selected_map_->image_info_file_name_.toStdString().c_str() ) == 0 )
    {
        qDebug() << map_image_info_.DebugString();
    }
    else
    {
        qDebug() << "failed to load image info!";
        return;
    }
    operation_btns_[kModifyMap]->setEnabled( has_map_ );

    // calculate first origin
    // scale the image
    double scaled_image_wth = (double)( image_size.width() ) * factor_;
    double scaled_image_hgt = (double)( image_size.height() ) * factor_;
    double scaled_image_left = ( view_wth - scaled_image_wth ) * 0.5;
    double scaled_image_top = ( view_hgt - scaled_image_hgt ) * 0.5;

    QPoint tmp_offset = origin_offset_ + origin_offset_single_move_;
    QPoint image_left_top( scaled_image_left + tmp_offset.x(), scaled_image_top + tmp_offset.y() );

    Vector2i tmp_origin_offset = map_image_info_.origin_ * factor_;
    origin_ = image_left_top + tmp_origin_offset;
    origin_offset_ = QPoint( 0, 0 );
    origin_offset_single_move_ = QPoint( 0, 0 );
    resolution_ = map_image_info_.resolution_;

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
    if( currentConnected() != 0 || add_point_mode_ == kDelete )
        return;

    PointWithInfo info_point;
    last_target_point_id_++;
    info_point.id = last_target_point_id_;

    if( add_point_mode_ == kInRobot )
    {
        current_selected_robot_->sendCommand_AddPoint();
        QPointF current_robot_pos( current_selected_robot_->state_.x, current_selected_robot_->state_.y );

        info_point.point = current_robot_pos;
    }
    else if( add_point_mode_ == kInUI )
    {
        current_selected_robot_->sendCommand_AddPoint( last_target_point_set_in_ui_.x(), last_target_point_set_in_ui_.y(), 0.);
        qDebug() << last_target_point_set_in_ui_;
        info_point.point = last_target_point_set_in_ui_;
    }

    target_points_.push_back( info_point );
    points_maybe_selected_.push_back( false );

    update();
}

void StatusMonitorView::slotOnSetLoopMode()
{
    if( currentConnected() != 0 )
        return;

    current_selected_robot_->sendCommand_SetLoopMode();
    need_restart_record_target_list_ = true;
}

void StatusMonitorView::slotOnSetReverseMode()
{
    if( currentConnected() != 0 )
        return;

    current_selected_robot_->sendCommand_SetReverseMode();
    need_restart_record_target_list_ = true;
}

int32_t StatusMonitorView::currentConnected()
{
    DisplayMessage msg;
    msg.level_ = kErrorMsg;

    if( !current_selected_robot_ )
    {
        msg.msg_ = "No Robot Selected!";
        msg_box_->setMessage( msg );
        return -1;
    }
    if( !current_selected_robot_->connected_ )
    {
        msg.msg_ = "Robot Not Connected!";
        msg_box_->setMessage( msg );
        return -1;
    }

    return 0;
}


