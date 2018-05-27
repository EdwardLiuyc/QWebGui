#ifndef ROBOTMANAGEMENTVIEW_H
#define ROBOTMANAGEMENTVIEW_H

#include <QWidget>
#include <QList>
#include <QTableView>
#include <QStandardItem>
#include <QPushButton>
#include <QAbstractTableModel>
#include "common_defines.h"

class Robot;
class SettingTableModel;

class RobotManagementView : public QWidget
{
    Q_OBJECT
public:
    explicit RobotManagementView(std::list<RobotSettings>* list, QWidget *parent = 0);

    enum SettingTableCol
    {
        kName,
        kIP,
        kPort,
        kTableColCount
    };

    enum Operations
    {
        kSave, kDelete, kOperationsCount
    };

protected:
    void resizeEvent(QResizeEvent* event);

    int32_t SaveToSettingFile( const char* filename );

signals:
    void signalReturn();

public slots:
    void slotOnReturnBtnClicked();
    void slotOnSaveBtnClicked();

private:
    std::list<RobotSettings>* robot_setting_list_;
    QTableView* robot_setting_table_;
    QPushButton* operation_btns_[kOperationsCount];

    QPushButton* return_btn_;
};



class SettingTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    SettingTableModel( QWidget* parent = 0 ) : QAbstractTableModel( parent ) {}
    ~SettingTableModel(){}

    void setSettingData( std::list<RobotSettings>* list ) { robot_setting_list_ = list; }
    Qt::ItemFlags flags(const QModelIndex & index) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);

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
        return RobotManagementView::kTableColCount;
    }

private:
    std::list<RobotSettings>* robot_setting_list_;
};

#endif // ROBOTMANAGEMENTVIEW_H
