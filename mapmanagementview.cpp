#include "mapmanagementview.h"
#include <QHeaderView>

MapManagementView::MapManagementView(std::list<MapSetting> *list, QWidget *parent)
    : QWidget(parent)
    , map_setting_dlg_( NULL )
    , map_setting_list_( list )
{
    QString operation_str[kOperationCount] = {"Add", "Modify","Delete"};
    for( int i = 0; i < kOperationCount; ++i )
    {
        operation_btns_[i] = new QPushButton(this);
        operation_btns_[i]->setText( operation_str[i] );
        operation_btns_[i]->setFont( SYSTEM_UI_FONT_10_BOLD );
        operation_btns_[i]->setFocusPolicy(Qt::NoFocus);
    }
    QObject::connect( operation_btns_[kAdd], SIGNAL(clicked()), this, SLOT(slotOnAddBtnClicked()));

    map_setting_table_ = new QTableView( this );
    map_table_model_ = new MapTableModel( map_setting_table_ );
    map_table_model_->setMapData( map_setting_list_ );
    map_setting_table_->setModel( map_table_model_ );
    map_setting_table_->horizontalHeader()->setSectionResizeMode( MapManagementView::kName ,QHeaderView::Stretch);

    return_btn_ = new QPushButton(this);
    return_btn_->setText("back");
    QPalette pal = return_btn_->palette();
    pal.setColor(QPalette::ButtonText,QColor(255,0,0));
    return_btn_->setPalette(pal);
    return_btn_->setFont( SYSTEM_UI_FONT_14_BOLD );
    QObject::connect( return_btn_, SIGNAL(clicked()), this, SLOT(slotOnReturnBtnClicked()));
}

void MapManagementView::slotOnReturnBtnClicked()
{
    this->hide();
    emit signalReturn();
}

void MapManagementView::slotOnModifyBtnClicked()
{

}

void MapManagementView::resizeEvent(QResizeEvent *event)
{
    int32_t view_wdt = this->width();
    int32_t view_hgt = this->height();

    int32_t btn_wdt = 140;
    int32_t btn_hgt = 40;
    int32_t gap_wdt = 20;
    int32_t gap_hgt = 10;
    int32_t table_wdt = view_wdt - btn_wdt - gap_wdt ;
    if( table_wdt > 700 )
        table_wdt = 700;

    int32_t table_left = ( view_wdt - gap_wdt - table_wdt - btn_wdt ) / 2;
    map_setting_table_->setGeometry( table_left, 0, table_wdt, view_hgt);
    int32_t path_wth = ( table_wdt - 150 ) / 2;;
    map_setting_table_->setColumnWidth( MapManagementView::kImagePath, path_wth );
    map_setting_table_->setColumnWidth( MapManagementView::kImageSettingPath, path_wth );

    for( int i = 0; i < kOperationCount; ++i )
    {
        operation_btns_[i]->setGeometry( table_left + table_wdt + gap_wdt,
                                         view_hgt - (kOperationCount-i)*btn_hgt - (kOperationCount-i-1)*gap_hgt,
                                         btn_wdt, btn_hgt );
    }

    return_btn_->setGeometry( table_left + table_wdt + gap_wdt, 0, btn_wdt, btn_hgt );

    QWidget::resizeEvent( event );
}

void MapManagementView::slotOnAddBtnClicked()
{
    if( !map_setting_dlg_ )
        map_setting_dlg_ = new AddMapSettingDlg( this );

    map_setting_dlg_->exec();
}





// model ******************
Qt::ItemFlags MapTableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return QAbstractItemModel::flags(index);

    return ( Qt::ItemIsEnabled | Qt::ItemIsSelectable );
}

QVariant MapTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if( index.row() >= (int32_t)map_setting_list_->size() )
        return QVariant();

    int row = index.row();
    int col = index.column();

    std::list<MapSetting>::iterator it = map_setting_list_->begin();
    std::advance( it, row );

    switch (role)
    {
    case Qt::TextColorRole:
        return QColor(Qt::black);
    case Qt::TextAlignmentRole:
        return QVariant(Qt::AlignCenter);
    case Qt::DisplayRole:
    {
        if( col == MapManagementView::kName )
            return it->name_;
        else if( col == MapManagementView::kImagePath )
            return it->image_file_name_;
        else if( col == MapManagementView::kImageSettingPath )
            return it->image_setting_file_name_;
        return "";
    }
    default:
        return QVariant();
    }

    return QVariant();
}

QVariant MapTableModel::headerData(int section, Qt::Orientation orientation, int role) const
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
            case MapManagementView::kName:
                header_str = QString("Name");
                break;
            case MapManagementView::kImagePath:
                header_str = QString("Image Path");
                break;
            case MapManagementView::kImageSettingPath:
                header_str = QString("Xml Path");
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


