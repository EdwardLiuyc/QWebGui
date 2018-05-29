#ifndef STATUSMONITORVIEW_H
#define STATUSMONITORVIEW_H

#include <QWidget>
#include <QPushButton>
#include <QTableView>
#include <QAbstractItemModel>
#include <QAbstractTableModel>
#include <list>
#include <QComboBox>
#include "robotselectview.h"

enum MonitorMode
{
    kStatusMonitorMode,
    kPathManageMode,
    kMonitorModeCount
};

class Robot;
class StatusMonitorView : public QWidget
{
    Q_OBJECT
public:
    explicit StatusMonitorView( std::list<Robot>* robots
                                , std::list<MapSetting>* maps
                                , QWidget *parent = 0, MonitorMode mode = kStatusMonitorMode);

    enum Operation
    {
        kSelectRobot,
        kSelectMap,
        kOperationCount,
    };

    using Vector2i = QPoint;

protected:
    void resizeEvent(QResizeEvent* event);
    void paintEvent(QPaintEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void wheelEvent(QWheelEvent* event);
    void timerEvent(QTimerEvent* event);

signals:
    void signalReturn();

public slots:
    void slotOnReturnBtnClicked();

    void slotOnSelectRobotBtnClicked( bool checked );
    void slotOnSelectMapBtnClicked( bool checked );

private:
    // ** widgets **
    QPushButton* operation_btns_[kOperationCount];
    QPushButton* return_btn_;
    RobotSelectView* robot_select_view_;
    QComboBox* map_select_box_;

    // ** robot data **
    std::list<Robot>* robots_;
    std::list<MapSetting>* maps_;

    // ** for paint **
    MonitorMode mode_;
    bool has_map_;
    bool got_first_origin_;
    double factor_;
    QPoint start_pos_;
    QPoint origin_;
    Vector2i origin_offset_;
    Vector2i origin_offset_single_move_;

    int32_t timer_update_robots_;
};

#endif // STATUSMONITORVIEW_H
