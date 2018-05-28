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

    using Vector2i = QPoint;

protected:
    void resizeEvent(QResizeEvent* event);
    void showEvent(QShowEvent* event);
    void paintEvent(QPaintEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);

    void updateRobotTableView();
    void paintACoordSystem(QPainter *painter, QPoint& org );



signals:
    void signalReturn();

public slots:
    void slotOnReturnBtnClicked();

    void slotOnSelectRobotBtnClicked();

private:
    // ** widgets **
    QPushButton* operation_btns_[kOperationCount];

    QPushButton* return_btn_;
    RobotSelectView* robot_select_view_;

    // ** robot data **
    std::list<Robot>* robots_;

    // ** for paint **
    bool has_map_;
    QPoint start_pos_;
    QPoint origin_;
    Vector2i origin_offset_;

};

#endif // STATUSMONITORVIEW_H
