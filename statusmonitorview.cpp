#include "statusmonitorview.h"
#include <QPainter>
#include <QMouseEvent>

StatusMonitorView::StatusMonitorView(std::list<Robot> *robots, QWidget *parent)
    : QWidget(parent)
    , robots_(robots)
    , has_map_( false )
    , got_first_origin_( false )
    , origin_()
    , origin_offset_()
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
    QObject::connect( operation_btns_[kSelectRobot], SIGNAL(clicked()), this, SLOT(slotOnSelectRobotBtnClicked()));

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

void StatusMonitorView::paintACoordSystem(QPainter *painter, QPoint &org)
{
    const int length = 30;
    if( org.x() < -length || org.y() < -length )
        return;

    QPoint left, right, top, btm;
    left.setX( org.x() - length );
    left.setY( org.y() );
    right.setX( org.x()+ length);
    right.setY( org.y());
    top.setX( org.x() );
    btm.setX( org.x() );
    top.setY( org.y() - length );
    btm.setY( org.y() + length );
    QPen pen( Qt::green, 2, Qt::DashDotLine, Qt::RoundCap, Qt::RoundJoin );
    painter->setPen(pen);
    painter->drawLine(left, right);
    painter->drawLine(top, btm);
}

void StatusMonitorView::paintEvent(QPaintEvent *event)
{
    QPainter painter( this );
    if( !has_map_ )
    {

        QPoint tmp_origin = origin_ + origin_offset_ + origin_offset_single_move_;
        paintACoordSystem( &painter, tmp_origin );
    }
    else
    {

    }


    QWidget::paintEvent( event );
}

void StatusMonitorView::slotOnReturnBtnClicked()
{
    this->hide();
    emit signalReturn();
}

void StatusMonitorView::slotOnSelectRobotBtnClicked()
{
    robot_select_view_->setVisible( operation_btns_[kSelectRobot]->isChecked() );

}
