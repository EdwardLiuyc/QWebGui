#include "robotmanagementview.h"
#include <QHeaderView>
#include <QColor>
#include "pugixml/pugiconfig.hpp"
#include "pugixml/pugixml.hpp"

RobotManagementView::RobotManagementView(std::list<RobotSettings> *list, QWidget *parent) : QWidget(parent)
{
    robot_setting_list_ = list;
    robot_setting_table_ = new QTableView( this );
    robot_setting_table_->setFont(QFont("SIMHEI",10,QFont::Bold));

    SettingTableModel* item_model = new SettingTableModel( robot_setting_table_ );
    item_model->setSettingData( robot_setting_list_ );

    robot_setting_table_->setModel( item_model );

    robot_setting_table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    robot_setting_table_->setColumnWidth( kName, 180 );
    robot_setting_table_->setColumnWidth( kPort, 80 );
    robot_setting_table_->horizontalHeader()->setSectionResizeMode( kIP,QHeaderView::Stretch);

    robot_setting_table_->setEditTriggers( QAbstractItemView::DoubleClicked );
    robot_setting_table_->verticalHeader()->setDefaultAlignment(Qt::AlignCenter);

    QString operations_str[kOperationsCount] = {"Save Config", "Delete"};
    for( int i = 0; i < kOperationsCount; ++i )
    {
        operation_btns_[i] = new QPushButton(this);
        operation_btns_[i]->setText( operations_str[i] );
        operation_btns_[i]->setFont( SYSTEM_UI_FONT_10_BOLD );
    }
    QObject::connect(operation_btns_[kSave], SIGNAL(clicked()), this, SLOT(slotOnSaveBtnClicked()));

    return_btn_ = new QPushButton(this);
    return_btn_->setText("back");
    QPalette pal = return_btn_->palette();
    pal.setColor(QPalette::ButtonText,QColor(255,0,0));
    return_btn_->setPalette(pal);
    return_btn_->setFont( QFont("system-ui",14,QFont::Bold,true) );
    QObject::connect( return_btn_, SIGNAL(clicked()), this, SLOT(slotOnReturnBtnClicked()));
}

void RobotManagementView::resizeEvent(QResizeEvent *event)
{
    int32_t view_wdt = this->width();
    int32_t view_hgt = this->height();

    int32_t btn_wdt = 140;
    int32_t btn_hgt = 40;
    int32_t gap_wdt = 20;
    int32_t gap_hgt = 10;
    int32_t table_wdt = view_wdt - btn_wdt - gap_wdt ;
    if( table_wdt > 800 )
        table_wdt = 800;

    int32_t table_left = ( view_wdt - gap_wdt - table_wdt - btn_wdt ) / 2;
    robot_setting_table_->setGeometry( table_left, 0, table_wdt, view_hgt);

    for( int i = 0; i < kOperationsCount; ++i )
    {
        operation_btns_[i]->setGeometry( table_left + table_wdt + gap_wdt,
                                         view_hgt - (kOperationsCount-i)*btn_hgt - (kOperationsCount-i-1)*gap_hgt,
                                         btn_wdt, btn_hgt );
    }

    return_btn_->setGeometry( table_left + table_wdt + gap_wdt, 0, btn_wdt, btn_hgt );

    QWidget::resizeEvent( event );
}

void RobotManagementView::slotOnReturnBtnClicked()
{
    this->hide();
    emit signalReturn();
}

void RobotManagementView::slotOnSaveBtnClicked()
{
    emit signalSaveSetting();
}

Qt::ItemFlags SettingTableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return QAbstractItemModel::flags(index);

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
    return flags;
}

QVariant SettingTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if( index.row() >= (int32_t)robot_setting_list_->size() )
        return QVariant();

    int row = index.row();
    int col = index.column();
    auto setting = robot_setting_list_->begin();
    std::advance(setting, row);

    switch (role)
    {
//    case Qt::TextColorRole:
//        return QColor(Qt::black);
    case Qt::TextAlignmentRole:
        return QVariant(Qt::AlignCenter);
    case Qt::DisplayRole:
    {
        QString str;
        switch(col)
        {
        case RobotManagementView::kName:
            str = setting->name_;
            break;
        case RobotManagementView::kIP:
            str = setting->ip_;
            break;
        case RobotManagementView::kPort:
            if( setting->port_ != 0 )
                str = QString::number( setting->port_ );
            else
                str = "";
            break;
        default:
            str = "";
            break;
        }
        return str;
    }
    default:
        return QVariant();
    }

    return QVariant();
}

QVariant SettingTableModel::headerData(int section, Qt::Orientation orientation, int role) const
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
            case RobotManagementView::kName:
                header_str = QString("Name");
                break;
            case RobotManagementView::kIP:
                header_str = QString("IP");
                break;
            case RobotManagementView::kPort:
                header_str = QString("Port");
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

bool SettingTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;

    while( index.row() >= (int32_t)robot_setting_list_->size() )
        robot_setting_list_->emplace_back();

    int row = index.row();
    int col = index.column();
    auto setting = robot_setting_list_->begin();
    std::advance(setting, row);

    switch (role)
    {
    case Qt::EditRole:
    {
        switch( col )
        {
        case RobotManagementView::kName:
            setting->name_ = value.toString();
            break;
        case RobotManagementView::kIP:
            setting->ip_ = value.toString();
            break;
        case RobotManagementView::kPort:
            setting->port_ = value.toInt();
            break;
        default:
            return false;
        }

        return true;
    }
    default:
        return false;
    }

    return false;
}

