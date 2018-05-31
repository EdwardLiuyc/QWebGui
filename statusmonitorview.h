#ifndef STATUSMONITORVIEW_H
#define STATUSMONITORVIEW_H

#include <QWidget>
#include <QPushButton>
#include <QTableView>
#include <QAbstractItemModel>
#include <QAbstractTableModel>
#include <list>
#include <QComboBox>
#include <QImageReader>
#include <QImage>
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

    enum PathMngOperation
    {
        kGetPoint,
        kSetReverseMode,
        kSetLoopMode,
        kPathMngOperationCount
    };

    enum PathMngMode
    {
        kOldMode,
        kSendAfterSaveAll,
        kPathMngModeCount
    };

    using Vector2i = QPoint;

    inline void setMode( MonitorMode mode ){ monitor_mode_ = mode; }

protected:
    void resizeEvent(QResizeEvent* event);
    void paintEvent(QPaintEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void wheelEvent(QWheelEvent* event);
    void timerEvent(QTimerEvent* event);

    void hideSwitchableWidgets();

    int32_t getImageInfoFromFile( MapImageInfo& info, const char* info_filename );

signals:
    void signalReturn();

public slots:
    void slotOnReturnBtnClicked();

    void slotOnSelectRobotBtnClicked( bool checked );
    void slotOnSelectMapBtnClicked( bool checked );
    void slotOnSwitchBtnClicked();
    void slotLoadMapImage( int index );
    void slotOnRobotSelected( int index );

private:
    // ** widgets **
    QPushButton* operation_btns_[kOperationCount];
    QPushButton* path_mng_btns_[kPathMngOperationCount];
    QPushButton* return_btn_;
    QPushButton* switch_btn_;
    RobotSelectView* robot_select_view_;
    QComboBox* map_select_box_;
    QComboBox* robot_select_box_;

    // ** robot data **
    std::list<Robot>* robots_;
    std::list<MapSetting>* maps_;
    MapSetting local_map_;
    Robot* current_selected_robot_;

    // ** for paint **
    MonitorMode monitor_mode_;
    PathMngMode path_manage_mode_;
    bool has_map_;
    bool got_first_origin_;
    double factor_;
    double min_factor_;
    QPoint start_pos_;
    QPoint origin_;
    Vector2i origin_offset_;
    Vector2i origin_offset_single_move_;

    QImage image_;
    MapImageInfo map_image_info_;

    int32_t timer_update_robots_;
};

#endif // STATUSMONITORVIEW_H
