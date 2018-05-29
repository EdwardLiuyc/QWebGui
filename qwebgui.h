#ifndef QWEBGUI_H
#define QWEBGUI_H

#include <QMainWindow>
#include <QPushButton>
#include <list>
#include "robotmanagementview.h"
#include "statusmonitorview.h"
#include "mapmanagementview.h"
#include "robot.h"

class QWebGui : public QMainWindow
{
    Q_OBJECT

public:

    enum MainBtnContent
    {
        kRobotManage,
        kMapManage,
        kPathManage,
        kStatusMonitor,
        kMainBtnCount
    };

    QWebGui(QWidget *parent = 0);
    ~QWebGui();

public slots:
    void slotOnRobotManageClicked();
    void slotOnRobotManageReturned();

    void slotOnMapManageClicked();
    void slotOnMapManageReturned();

    void slotOnPathManageClicked();
    void slotOnPathManageReturned();

    void slotOnStatusMonitorClicked();
    void slotOnStatusMonitorReturned();

protected:
    void resizeEvent(QResizeEvent* event);
    void updateRobotListFromSettings();

    int32_t initWithSettingFile(const char* filename);

private:
    QPushButton* main_btns_[kMainBtnCount];
    RobotManagementView* robot_manage_view_;
    StatusMonitorView* status_monitor_view_;
    MapManagementView* map_manage_view_;

    // All data
    std::list<RobotSettings> robot_setting_list_;
    std::list<Robot> robots_;
    std::list<MapSetting> map_setting_list_;
};

#endif // QWEBGUI_H
