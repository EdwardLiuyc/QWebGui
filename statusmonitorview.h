#ifndef STATUSMONITORVIEW_H
#define STATUSMONITORVIEW_H

#include <QWidget>
#include <QPushButton>
#include <QTableView>
#include <QAbstractItemModel>
#include <QAbstractTableModel>
#include <list>
#include "robotselectview.h"

class Robot;
class StatusMonitorView : public QWidget
{
    Q_OBJECT
public:
    explicit StatusMonitorView( std::list<Robot>* robots, QWidget *parent = 0);

    enum Operation
    {
        kSelectRobot,
        kSelectMap,
        kOperationCount,
    };

protected:
    void resizeEvent(QResizeEvent* event);
    void showEvent(QShowEvent* event);

    void updateRobotTableView();

signals:
    void signalReturn();

public slots:
    void slotOnReturnBtnClicked();

    void slotOnSelectRobotBtnClicked();

private:
    QPushButton* operation_btns_[kOperationCount];

    QPushButton* return_btn_;
    RobotSelectView* robot_select_view_;
    std::list<Robot>* robots_;
};

#endif // STATUSMONITORVIEW_H
