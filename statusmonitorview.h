#ifndef STATUSMONITORVIEW_H
#define STATUSMONITORVIEW_H

#include <QWidget>
#include <QPushButton>
#include <QTableView>
#include <QAbstractItemModel>
#include <QAbstractTableModel>
#include <QOpenGLWidget>
#include <list>
#include <QComboBox>
#include <QImageReader>
#include <QImage>
#include <QCheckBox>
#include "robotselectview.h"
#include "msgbox.h"

enum MonitorMode
{
    kStatusMonitorMode,
    kPathManageMode,
    kMonitorModeCount
};

using Vector2i = QPoint;
using Vector2F = QPointF;
class Robot;
class StatusMonitorView : public QOpenGLWidget
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
        kChangeAddPointMode,
        kOperationCount,
    };

    enum PathMngOperation
    {
        kAddPoint,
        kSetReverseMode,
        kSetLoopMode,
        kPathMngOperationCount
    };

    enum AddPointMode
    {
        kInRobot, kInUI
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

    void initializeGL() override;
    void paintGL() override;

    void hideSwitchableWidgets();

    int32_t getImageInfoFromFile( MapImageInfo& info, const char* info_filename );

signals:
    void signalReturn();

public slots:
    void slotOnReturnBtnClicked();

    void slotOnSelectRobotBtnClicked( bool checked );
    void slotOnSelectMapBtnClicked( bool checked );
    void slotOnChangeRunningModeClicked( bool checked );
    void slotOnRunOrHaltBtnClicked( bool checked );
    void slotOnChangeAddPointModeClicked( bool checked );
    void slotOnSwitchBtnClicked();
    void slotOnMapSelected( int index );
    void slotOnRobotSelected( int index );
    void slotOnRcvCurrentRobotMsg( DisplayMessage& msg );

    void slotOnAddPoint();
    void slotOnSetReverseMode();
    void slotOnSetLoopMode();

private:
    // ** widgets **
    QPushButton*    operation_btns_[kOperationCount];
    QPushButton*    path_mng_btns_[kPathMngOperationCount];
    QPushButton*    return_btn_;
    QPushButton*    switch_btn_;
    RobotSelectView* robot_select_view_;
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
    AddPointMode add_point_mode_ = kInRobot;
    RobotRunningMode robot_running_mode_ = RobotRunningMode::kManual;
    QPointF set_point_in_ui_;
    bool has_map_;
    bool got_first_origin_;
    double factor_;
    double min_factor_;
    double resolution_;
    QPoint start_pos_;
    QPoint origin_;
    Vector2i origin_offset_;
    Vector2i origin_offset_single_move_;

    QImage image_;
    MapImageInfo map_image_info_;

    int32_t timer_update_robots_;
    int32_t timer_manual_operating_;

    // for manual operation
    enum DirKey
    {
        kUp, kDown, kLeft, kRight, kKeyCount
    };
    bool key_pressed_[DirKey::kKeyCount];
    double manual_strength_     = 0.;
    double manual_angle_        = 0.;
    Vector2F manual_vec_;
    bool have_manual_stop_;
};

#endif // STATUSMONITORVIEW_H
