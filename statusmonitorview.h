#ifndef STATUSMONITORVIEW_H
#define STATUSMONITORVIEW_H

#include <QWidget>
#include <QPushButton>
#include <QTableView>
#include <QAbstractItemModel>
#include <QAbstractTableModel>
#include <list>
#include <vector>
#include <QMutex>
#include <QComboBox>
#include <QImageReader>
#include <QImage>
#include <QCheckBox>
#include <QtMath>
#include "robotselectview.h"
#include "msgbox.h"
#include "robotstatusview.h"

enum MonitorMode
{
    kStatusMonitorMode,
    kPathManageMode,
    kMonitorModeCount
};

using Vector2i = QPoint;
using Vector2F = QPointF;
class Robot;
class StatusMonitorView : public QWidget
{
    Q_OBJECT
public:
    explicit StatusMonitorView( std::list<Robot>* robots
                                , std::list<MapSetting>* maps
                                , QWidget *parent = 0, MonitorMode mode = kStatusMonitorMode);
    ~StatusMonitorView();

    enum Operation
    {
        kSelectRobot,
        kSelectMap,
        kChangeRunningMode,
        kRunOrHalt,
        kManagePoints,
        kModifyMap,
        kManagePath,
        kOperationCount,
    };

    enum PathMngOperation
    {
        kAddPoint,
        kSetReverseMode,
        kSetLoopMode,
        kPathMngOperationCount
    };

    enum PointsMngOperation
    {
        kChangeAddPointMode,
        kDeletePoint,
        kPointsMngOperationCount
    };

    enum ModifyMapOperations
    {
        kStartSelectPoints,
        kFinishSelectPoints,
        kDeleteArea,
        kSaveToMap,
        kModifyMapOpCount
    };

    enum ModifyMapState
    {
        kDoingNothing,
        kAddingPoints,
        kDeletingArea
    };

    enum AddPointMode
    {
        kInRobot, kInUI, kDelete
    };

    enum PathMngSubOperation
    {
        kConnectPoints,
        kPickBeginAndEnd,
        kModifySinglePath,
        kPathMngSubOperationCount
    };

    inline void setMode( MonitorMode mode ){ monitor_mode_ = mode; }

protected:
    void resizeEvent(QResizeEvent* event);
    void paintEvent(QPaintEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void wheelEvent(QWheelEvent* event);
    void timerEvent(QTimerEvent* event);
    void keyPressEvent(QKeyEvent* event);
    void keyReleaseEvent(QKeyEvent *event);

    void hideSwitchableWidgets();
    int32_t currentConnected();
    void addConnectionToPoints();
    int32_t getImageInfoFromFile( MapImageInfo& info, const char* info_filename );
    int32_t findTheNearestPoint(QPointF& src_point, QList<PointWithInfo> &dst_points, double* dis );

signals:
    void signalReturn();

public slots:
    void slotOnReturnBtnClicked();

    void slotOnSelectRobotBtnClicked( bool checked );
    void slotOnSelectMapBtnClicked( bool checked );
    void slotOnChangeRunningModeClicked( bool checked );
    void slotOnRunOrHaltBtnClicked( bool checked );
    void slotOnPointsManageClicked( bool checked );
    void slotOnModifyMapClicked( bool checked );
    void slotOnManagePathClicked( bool checked );
    void slotOnSwitchBtnClicked();
    void slotOnMapSelected( int index );
    void slotOnRobotSelected( int index );
    void slotOnRcvCurrentRobotMsg( DisplayMessage& msg );

    void slotHandleModifyMapBtns();
    void slotHandlePointsMngBtns();
    void slotHandlePathMngSubBtns();

    void slotOnAddPoint();
    void slotOnSetReverseMode();
    void slotOnSetLoopMode();

private:
    // ** widgets **
    QPushButton*    operation_btns_[kOperationCount];
    QPushButton*    path_mng_btns_[kPathMngOperationCount];
    QPushButton*    point_mng_sub_btns_[kPointsMngOperationCount];
    QPushButton*    modify_map_sub_btns_[kModifyMapOpCount];
    QPushButton*    path_mng_sub_btns_[kPathMngSubOperationCount];

    QPushButton*    return_btn_;
    QPushButton*    switch_btn_;
    RobotSelectView* robot_select_view_;
    RobotStatusView* robot_status_view_;
    QComboBox*      map_select_box_;
    QComboBox*      robot_select_box_;
    MsgBox*         msg_box_;

    // ** robot data **
    std::list<Robot>* robots_;
    std::list<MapSetting>* maps_;
    MapSetting*     current_selected_map_;
    Robot*          current_selected_robot_;

    // ** for paint **
    MonitorMode monitor_mode_;
    PathMngMode path_manage_mode_;
    AddPointMode add_point_mode_            = AddPointMode::kInRobot;
    RobotRunningMode robot_running_mode_    = RobotRunningMode::kManual;
    ModifyMapState  modify_map_state_       = ModifyMapState::kDoingNothing;
    QPointF last_target_point_set_in_ui_;

    QList<PointWithInfo> target_points_;
    int32_t last_target_point_id_ = -1;
    QList<bool> points_maybe_selected_;
    QList<NodeInPath> path_manage_nodes_;
    bool need_restart_record_target_list_;
    bool has_map_;
    bool got_first_origin_;
    bool is_managing_points_;
    double factor_;
    double min_factor_;
    double resolution_;
    QPoint start_pos_for_mouse_move_;

    // ** modify the map ( add obstacle area ) **
    QMutex mutex_;
    bool is_modifying_map_ = false;
    QList<QPointF> current_adding_points_;
    QList<QPointF> tmp_show_current_adding_points_;
    QList< QList<QPointF> > modifyed_points_sets_;
    QList<bool> selected_to_delete_;

    // ** origin **
    QPoint      origin_;
    Vector2i    origin_offset_;
    Vector2i    origin_offset_single_move_;

    QImage image_;
    MapImageInfo map_image_info_;

    int32_t timer_update_robots_;
    int32_t timer_manual_operating_;

    // ** for manual operation **
    enum DirKey
    {
        kUp, kDown, kLeft, kRight, kKeyCount
    };
    bool key_pressed_[DirKey::kKeyCount];
    double manual_strength_     = 0.;
    double manual_angle_        = 0.;
    Vector2F manual_vec_;
    bool have_manual_stop_;

    // ** path manage **
    bool is_managing_path_;
    enum PathManageState
    {
        PICKING_FIRST_POINT,
        PICKING_CONNECTING_POINT,
        PICKING_PATH_START,
        PICKING_PATH_END,
        PICKING_MODIFYING_PATH
    };
    PathManageState path_manage_state_;
    QList<Path> paths_;
    int32_t current_first_point_index_          = -1;
    int32_t current_connecting_point_index_     = -1;
    QPointF tmp_show_line_end_;
};

#endif // STATUSMONITORVIEW_H
