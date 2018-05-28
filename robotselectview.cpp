#include "robotselectview.h"
#include <QHeaderView>
#include <QTimerEvent>

RobotSelectView::RobotSelectView(std::list<Robot> *list, QWidget *parent)
    : QWidget(parent)
    , robots_list_( list )
{
    initRobotTableView();
    QString operation_str[kOperationCount] = {"Select\nAll", "Close\nSelected","Connect\nSelected"};
    for( int i = 0; i < kOperationCount; ++i )
    {
        operation_btns_[i] = new QPushButton(this);
        operation_btns_[i]->setText( operation_str[i] );
        operation_btns_[i]->setFont( SYSTEM_UI_FONT_10_BOLD );
        operation_btns_[i]->setFocusPolicy(Qt::NoFocus);
    }
    QObject::connect( operation_btns_[kSelectAll], SIGNAL(clicked()), this, SLOT(slotSelectAllBtnClicked()));
    QObject::connect( operation_btns_[kClose], SIGNAL(clicked()), this, SLOT(slotCloseBtnClicked()));
    QObject::connect( operation_btns_[kConnect], SIGNAL(clicked()), this, SLOT(slotConnectBtnClicked()));

    timer_for_reset_model_ = startTimer( 1000 );
}

void RobotSelectView::resizeEvent(QResizeEvent *event)
{
    int view_wdt = this->width();
    int view_hgt = this->height();

    int32_t btn_wdt = 100;
    int32_t btn_hgt = 50;
    int32_t gap_wdt = 10;
    int32_t gap_hgt = 10;
    int32_t btn_top = view_hgt - btn_hgt;
    for( int i = 0; i < kOperationCount; ++i )
    {
        operation_btns_[i]->setGeometry( view_wdt - (kOperationCount-i)*(btn_wdt+gap_wdt) + gap_wdt, btn_top, btn_wdt, btn_hgt);
    }

    int32_t table_hgt = btn_top - gap_hgt;
    robots_table_->setGeometry( 0, 0, view_wdt, table_hgt);

    QWidget::resizeEvent( event );
}

void RobotSelectView::timerEvent(QTimerEvent *event)
{
    if( isVisible() )
    {
        if( timer_for_reset_model_ == event->timerId() )
            table_model_->resetData();
    }

    QWidget::timerEvent( event );
}

void RobotSelectView::initRobotTableView()
{
    robots_table_ = new QTableView( this );

    robots_table_->setFont(QFont("SIMHEI",10,QFont::Bold));
    table_model_ = new RobotTableModel( robots_table_ );
    table_model_->setRobotData( robots_list_ );
    robots_table_->setModel( table_model_ );
    robots_table_->setColumnWidth( kNumber, 30 );
    robots_table_->setColumnWidth( kConnectionStatus, 50 );
    robots_table_->setColumnWidth( kSelectCheck, 30 );
    robots_table_->horizontalHeader()->setSectionResizeMode( kName,QHeaderView::Stretch);

    robots_table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    robots_table_->horizontalHeader()->setStretchLastSection(true);
    robots_table_->horizontalHeader()->setHighlightSections(false);
    robots_table_->verticalHeader()->hide();
    robots_table_->setShowGrid(false);
    robots_table_->setFrameShape(QFrame::NoFrame);
    robots_table_->setSelectionMode(QAbstractItemView::SingleSelection);

}

void RobotSelectView::slotSelectAllBtnClicked()
{
    if( robots_list_ )
    {
        for( Robot& robot: *robots_list_ )
            robot.selected_for_connect_ = true;
    }
    table_model_->resetData();
}

void RobotSelectView::slotConnectBtnClicked()
{
    if( robots_list_ )
    {
        for( Robot& robot: *robots_list_ )
            if( robot.selected_for_connect_ )
                robot.connectSocket();
    }
}

void RobotSelectView::slotCloseBtnClicked()
{
    if( robots_list_ )
        for( Robot& robot: *robots_list_ )
            if( robot.selected_for_connect_ )
                robot.disconnectSocket();
}

Qt::ItemFlags RobotTableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return QAbstractItemModel::flags(index);

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    if (index.column() == kSelectCheck)
        flags |= Qt::ItemIsUserCheckable;

    return flags;
}

QVariant RobotTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if( index.row() >= (int32_t)robots_list_->size() )
        return QVariant();

    int row = index.row();
    int col = index.column();

    std::list<Robot>::iterator robot = robots_list_->begin();
    std::advance( robot, row );

    switch (role)
    {
    case Qt::TextColorRole:
        return QColor(Qt::black);
    case Qt::TextAlignmentRole:
        return QVariant(Qt::AlignCenter);
    case Qt::DisplayRole:
    {
        if( col == kName )
            return robot->settings_.name_;
        else if( col == kConnectionStatus )
            return ( robot->connected_ ? QString("yes") : QString("no"));
        else if( col == kNumber )
            return QString::number(row+1);
        return "";
    }
    case Qt::CheckStateRole:
    {
        if( col == kSelectCheck )
            return robot->selected_for_connect_ ? Qt::Checked : Qt::Unchecked;
    }
    default:
        return QVariant();
    }

    return QVariant();
}

QVariant RobotTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    switch (role)
    {
    case Qt::TextAlignmentRole:
        return QVariant( Qt::AlignCenter );
    case Qt::DisplayRole:
    {
        if (orientation == Qt::Horizontal)
        {
            QString header_str;
            switch( section )
            {
            case kNumber:
                header_str = QString("No.");
                break;
            case kName:
                header_str = QString("Name");
                break;
            case kConnectionStatus:
                header_str = QString("C");
                break;
            case kSelectCheck:
                header_str = QString("S");
                break;
            default:
                return QVariant();
            }
            return header_str;
        }
        else if( orientation == Qt::Vertical )
        {
            return QString::number(section+1);
        }

    }
    default:
        return QVariant();
    }

    return QVariant();
}

bool RobotTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;

    if( index.row() >= (int32_t)robots_list_->size() )
        return false;

    int row = index.row();
    int col = index.column();

    std::list<Robot>::iterator robot = robots_list_->begin();
    std::advance( robot, row );

    bool ret = true;
    switch (role)
    {
    case Qt::DisplayRole:
    {
        ret = true;
        break;
    }
    case Qt::CheckStateRole:
    {
        if( col == kSelectCheck )
        {
            robot->selected_for_connect_ = (value.toInt() == Qt::Checked);
            ret = true;
        }
        break;
    }
    default:
        ret = false;
        break;
    }

    return ret;
}

