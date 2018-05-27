#ifndef ROBOTSELECTVIEW_H
#define ROBOTSELECTVIEW_H

#include <QWidget>
#include <QAbstractTableModel>
#include <QTableView>
#include "common_defines.h"
#include "robot.h"

enum RobotTableContent
{
    kNumber, kName, kConnectionStatus, kSelectCheck, kTableContentCount
};

class RobotTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    RobotTableModel( QWidget* parent = 0 ) : QAbstractTableModel( parent ) {}
    ~RobotTableModel(){}

    void setRobotData( std::list<Robot>* list ) { robots_list_ = list; }
    Qt::ItemFlags flags(const QModelIndex & index) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
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
        return kTableContentCount;
    }

private:
    std::list<Robot>* robots_list_;
};

class RobotSelectView : public QWidget
{
    Q_OBJECT
public:
    explicit RobotSelectView(std::list<Robot>* list, QWidget *parent = 0);

    enum Operations
    {
        kSelectAll, kConnect, kOperationCount
    };

protected:
    void initRobotTableView();
    void resizeEvent(QResizeEvent* event);
    void timerEvent(QTimerEvent* event);

signals:

public slots:
    void slotSelectAllBtnClicked();
    void slotConnectBtnClicked();

private:
    QPushButton* operation_btns_[kOperationCount];
    std::list<Robot>* robots_list_;

    RobotTableModel* table_model_;
    QTableView*  robots_table_;

    int32_t timer_for_reset_model_;
};

#endif // ROBOTSELECTVIEW_H
