#include "statusmonitorview.h"

StatusMonitorView::StatusMonitorView(std::list<Robot> *robots, QWidget *parent)
    : QWidget(parent)
    , robots_(robots)
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

    int32_t btn_wdt = 140;
    int32_t btn_hgt = 40;
    int32_t gap_wdt = 20;
    int32_t gap_hgt = 10;

    for( int i = 0; i < kOperationCount; ++i )
        operation_btns_[i]->setGeometry( gap_wdt, i*(btn_hgt+gap_hgt), btn_wdt, btn_hgt );

    return_btn_->setGeometry( gap_wdt, view_hgt - btn_hgt, btn_wdt, btn_hgt );

    int32_t table_left = btn_wdt + gap_wdt * 2;
    int32_t table_wdt = ( view_wdt * 0.4 ) ;
    table_wdt = table_wdt > 600 ? 600 : table_wdt;
    robot_select_view_->setGeometry( table_left, 0, table_wdt, view_hgt * 0.7);

    QWidget::resizeEvent( event );
}

void StatusMonitorView::showEvent(QShowEvent *event)
{
    updateRobotTableView();

    QWidget::showEvent( event );
}

void StatusMonitorView::slotOnReturnBtnClicked()
{
    this->hide();
    emit signalReturn();
}

void StatusMonitorView::updateRobotTableView()
{
}

void StatusMonitorView::slotOnSelectRobotBtnClicked()
{
    robot_select_view_->setVisible( operation_btns_[kSelectRobot]->isChecked() );

}
