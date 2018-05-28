#ifndef MAPMANAGEMENTVIEW_H
#define MAPMANAGEMENTVIEW_H

#include <QWidget>
#include <QPushButton>
#include <QTableView>
#include <QAbstractTableModel>
#include "common_defines.h"
#include "addmapsettingdlg.h"

class MapTableModel;
class MapManagementView : public QWidget
{
    Q_OBJECT
public:
    explicit MapManagementView( std::list<MapSetting>* list, QWidget *parent = 0 );
    enum Operations
    {
        kAdd, kModify, kDelete, kOperationCount
    };
    enum MapTableContect
    {
        kName, kImagePath, kImageSettingPath, kTableContentCount
    };

protected:
    void resizeEvent( QResizeEvent* event );

signals:
    void signalReturn();

public slots:
    void slotOnReturnBtnClicked();
    void slotOnAddBtnClicked();
    void slotOnModifyBtnClicked();

private:
    QPushButton* operation_btns_[kOperationCount];
    AddMapSettingDlg* map_setting_dlg_;

    MapTableModel* map_table_model_;
    QTableView* map_setting_table_;

    QPushButton* return_btn_;
    std::list<MapSetting>* map_setting_list_;
};

class MapTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    MapTableModel( QWidget* parent = 0 ) : QAbstractTableModel( parent ) {}
    ~MapTableModel(){}

    void setMapData( std::list<MapSetting>* list ) { map_setting_list_ = list; }
    Qt::ItemFlags flags(const QModelIndex & index) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    void resetData()
    {
        beginResetModel();
        endResetModel();
    }

    QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const
    {
        (void)parent;
        return createIndex( row, column );
    }

    int rowCount(const QModelIndex &) const
    {
        return 100;
    }

    int columnCount(const QModelIndex &) const
    {
        return MapManagementView::kTableContentCount;
    }

private:
    std::list<MapSetting>* map_setting_list_;
};

#endif // MAPMANAGEMENTVIEW_H
