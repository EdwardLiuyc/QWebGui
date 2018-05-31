#include "statusmonitorview.h"
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>


#define SWITCH_TO_MONITOR       QString( "Switch To\nMonitor" )
#define SWITCH_TO_PATHMANAGER   QString( "Switch To\nPathManage" )
StatusMonitorView::StatusMonitorView(std::list<Robot> *robots, std::list<MapSetting> *maps, QWidget *parent, MonitorMode mode)
    : QWidget( parent )
    , robots_( robots )
    , maps_( maps )
    , mode_( mode )
    , has_map_( false )
    , got_first_origin_( false )
    , factor_( 1.0 )
    , origin_()
    , origin_offset_()
    , timer_update_robots_( startTimer(500) )
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

    // ** return button **
    return_btn_ = new QPushButton(this);
    return_btn_->setText("back");
    return_btn_->setFocusPolicy( Qt::NoFocus );
    return_btn_->setFont( SYSTEM_UI_FONT_14_BOLD );
    QObject::connect( return_btn_, SIGNAL(clicked()), this, SLOT(slotOnReturnBtnClicked()));

    // ** switch button **
    switch_btn_ = new QPushButton( this );
    if( mode_ == MonitorMode::kStatusMonitorMode )
        switch_btn_->setText(SWITCH_TO_PATHMANAGER);
    else
        switch_btn_->setText(SWITCH_TO_MONITOR);

    switch_btn_->setFocusPolicy( Qt::NoFocus );
    switch_btn_->setFont( SYSTEM_UI_FONT_14_BOLD );
    QObject::connect( switch_btn_, SIGNAL(clicked()), this, SLOT(slotOnSwitchBtnClicked()));

    map_select_box_ = new QComboBox( this );
    map_select_box_->setVisible( false );
    map_select_box_->setFont( SYSTEM_UI_FONT_10_BOLD );
    QObject::connect( map_select_box_, SIGNAL(currentIndexChanged(int)), this, SLOT(slotLoadMapImage(int)));

    robot_select_box_ = new QComboBox( this );
    robot_select_box_->setVisible( false );
    robot_select_box_->setFont( SYSTEM_UI_FONT_10_BOLD );
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
        // paint background
        QColor bg_color( image_.pixel( 0, 0 ) );
        painter.setBrush(bg_color);
        painter.setPen(Qt::NoPen);
        painter.drawRect(rect());

        // has map file, show map image and all robotics run in the map
        QPixmap pixmap = QPixmap::fromImage( image_ );
        // scale the image
        QSize image_size = image_.size();

        int32_t scaled_image_wth = image_size.width() * factor_;
        int32_t scaled_image_hgt = image_size.height() * factor_;

        QRect target( 0, 0, scaled_image_wth, scaled_image_hgt );
        // draw the image
        painter.drawPixmap(target, pixmap, image_.rect());
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

void StatusMonitorView::slotOnSwitchBtnClicked()
{
    hideSwitchableWidgets();
    mode_ = ( mode_ == kStatusMonitorMode ? kPathManageMode : kStatusMonitorMode );
    if( mode_ == kStatusMonitorMode )
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

void StatusMonitorView::slotOnSelectRobotBtnClicked(bool checked)
{
    if( mode_ == MonitorMode::kStatusMonitorMode )
    {
        robot_select_view_->setVisible( checked );
        if( checked && operation_btns_[kSelectMap]->isChecked())
            operation_btns_[kSelectMap]->setChecked( false );
    }
    else
    {
        robot_select_box_->setVisible( checked );
        robot_select_box_->clear();
        QStringList robot_names;
        robot_names.append( QString("Select Robot...") );
        if( !robots_->empty() )
            for( Robot& robot : *robots_ )
                robot_names.append( robot.settings_.name_);
        robot_select_box_->addItems( robot_names );
    }
}

void StatusMonitorView::slotOnSelectMapBtnClicked(bool checked)
{
    if( mode_ == MonitorMode::kStatusMonitorMode && checked && operation_btns_[kSelectRobot]->isChecked() )
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

void StatusMonitorView::hideSwitchableWidgets()
{
    for( int i = 0; i < kOperationCount; ++i )
        operation_btns_[i]->setChecked( false );
}

void StatusMonitorView::slotLoadMapImage( int index )
{
    if( index == 0 )
        return;

    QImageReader reader;
    std::list<MapSetting>::iterator it = maps_->begin();
    std::advance( it, index - 1);

    if( local_map_.image_file_name_ == it->image_file_name_ )
        return;

    local_map_ = *it;
    reader.setFileName( local_map_.image_file_name_ );
    if( reader.canRead() )
    {
        qDebug() << "read image: " << local_map_.image_file_name_;
        image_ = reader.read();
        reader.setFormat("pgm");

        QStringList keys = reader.textKeys();
        QString strValue("");
        foreach (QString strKey, keys)
        {
            strValue = reader.text(strKey).toLocal8Bit();
            qDebug() << QString("key : %1  value : %2").arg(strKey).arg(strValue);
        }

        QSize image_size = image_.size();
        int32_t view_wth = this->width() - 2;
        int32_t view_hgt = this->height() - 1;
        int32_t image_wth = image_size.width();
        int32_t image_hgt = image_size.height();

        // the image is smaller than view
        double factor_x = (double)view_wth / (double)image_wth;
        double factor_y = (double)view_hgt / (double)image_hgt;
        factor_ = std::min( factor_x, factor_y );
        has_map_ = true;
    }
    else
    {
        QString strError = reader.errorString();
        qDebug() << "Read Error : " << strError;
    }
}
