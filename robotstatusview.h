#ifndef ROBOTSTATUSFRAMEWIDGET_H
#define ROBOTSTATUSFRAMEWIDGET_H

#include <QWidget>
#include <QGroupBox>
#include <QLabel>
#include <QGridLayout>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include "robot.h"
#include "common_defines.h"

class Robot;
class RobotStatusView : public QWidget
{
    Q_OBJECT
public:
    explicit RobotStatusView(QWidget *parent = 0);

    enum SubModules
    {
        kOverall,
        kGps,
        kSubmoduleCount
    };

    void setCurrentRobot( Robot* robot );

protected:
    void resizeEvent( QResizeEvent* event );
    void timerEvent( QTimerEvent* event );
    void showEvent( QShowEvent* event );
    void hideEvent( QHideEvent* event );

    void updateStatus();

signals:

public slots:

private:
    QGroupBox* submodule_boxes_[kSubmoduleCount];
    QGridLayout* submodule_layouts_[kSubmoduleCount];

    // Over All
    enum OverAllLabel
    {
        kConnected, kConnectedStatusValue,
        kX, kXValue, kY, kYValue, KYaw, KYawValue,
        kBattery, KBatteryValue,
        kErr, kErrValue,
        kOverallLabelCount
    };
    QLabel* overall_lbls_[OverAllLabel::kOverallLabelCount];

    // GPS
    enum GPSLabel
    {
        kGpsX, kGpsXValue, kGpsY, kGpsYValue, kGpsZ, KGpsZValue, kGpsMode, kGpsModeValue, kGpsLabelCount
    };
    QLabel* gps_lbls_[GPSLabel::kGpsLabelCount];

    Robot* current_robot_;

    QGraphicsOpacityEffect* opacity_;
    QPropertyAnimation*     animation_;

    int32_t timer_to_update_status_;
};

#endif // ROBOTSTATUSFRAMEWIDGET_H
