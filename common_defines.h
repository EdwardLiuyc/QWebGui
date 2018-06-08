#ifndef COMMON_DEFINES_H
#define COMMON_DEFINES_H

#include <QFont>
#include <QPushButton>
#include <list>
#include <queue>
#include <QPainter>

#define SYSTEM_UI_FONT_8   QFont("system-ui",8,QFont::Light)
#define SYSTEM_UI_FONT_10   QFont("system-ui",10,QFont::Light)
#define SYSTEM_UI_FONT_12   QFont("system-ui",12,QFont::Light)
#define SYSTEM_UI_FONT_14   QFont("system-ui",14,QFont::Light)
#define SYSTEM_UI_FONT_16   QFont("system-ui",16,QFont::Light)
#define SYSTEM_UI_FONT_18   QFont("system-ui",18,QFont::Light)

#define SYSTEM_UI_FONT_8_BOLD   QFont("system-ui",8,QFont::Bold)
#define SYSTEM_UI_FONT_10_BOLD   QFont("system-ui",10,QFont::Bold)
#define SYSTEM_UI_FONT_12_BOLD   QFont("system-ui",12,QFont::Bold)
#define SYSTEM_UI_FONT_14_BOLD   QFont("system-ui",14,QFont::Bold)
#define SYSTEM_UI_FONT_16_BOLD   QFont("system-ui",16,QFont::Bold)
#define SYSTEM_UI_FONT_18_BOLD   QFont("system-ui",18,QFont::Bold)

#define DOUBLE_EQUAL( a, b )    ( fabs( (a)-(b) ) < 1.e-6 )

struct RobotSettings
{
    QString     name_;
    QString     ip_;
    int32_t     port_;

    QString DebugString()
    {
        return name_ + " - " + ip_ + ":" + QString::number(port_);
    }
};

struct MapSetting
{
    QString name_;
    QString image_file_name_;
    QString image_info_file_name_;

    QString DebugString()
    {
        return name_ + "\n" + image_file_name_ + "\n" + image_info_file_name_;
    }
};

struct MapImageInfo
{
    QPoint origin_;
    double resolution_;
    QString unit_;

    QString DebugString()
    {
        return "origin: " + QString::number( origin_.x() ) + " : " + QString::number( origin_.y() )
                + " resulotion: " + QString::number( resolution_, 'd', 6 ) + unit_;
    }
};

enum PathMngMode
{
    kOldMode,
    kSendAfterSaveAll,
    kPathMngModeCount
};

enum AddPointType
{
    kNormal = 1,
    kRecover = 3,
    kCharge = 4
};

enum RobotRunningMode
{
    kManual = 0,
    kAuto = 1
};

enum MsgLevel
{
    kNormalMsg,
    kWarningMsg,
    kErrorMsg,
    kMsgLevelCount
};

struct DisplayMessage
{
    QString msg_;
    MsgLevel level_;

    DisplayMessage()
        : msg_()
        , level_( kNormalMsg )
    {}

    DisplayMessage( QString msg, MsgLevel level = kNormalMsg )
        : msg_( msg )
        , level_( level )
    {}
};

struct GpsPosition
{
    double x, y, z;
    int32_t mode;
};

struct RobotState
{
    double x, y;
    double yaw;
    double battery;
    int32_t error;

    GpsPosition gps_pos;
};


void paintACoordSystem(QPainter *painter, QPoint& org );
void PaintARobot(QPainter* painter, QPointF pos, double yaw, double factor);
void PaintADot(QPainter* painter, QPointF& pos );
void PaintATargetPoint( QPainter* painter, QPointF& pos );
void PaintRunningHistory( QPainter* painter, std::list<RobotState>& history, double robot_width, double resolution, double factor, QPoint origin );
void PaintASelectedArea( QPainter* painter, QList<QPointF>& points );
QPointF CalculateScreenPos( QPointF robot_pos, double resolution, QPoint origin, double factor );
QPointF CalculateRobotPos( QPointF screen_pos, double resolution, QPoint origin, double factor );
bool IsInsidePoly( const QPointF &iPoint, const QList<QPointF> &polygon );

#ifdef Q_OS_LINUX
// create a setting file in current directory
#define DEFAULT_SETTING_FILE "robot_settings.xml"
#else
#define DEFAULT_SETTING_FILE "C:\\robot_settings.xml"
#endif

#endif // COMMON_DEFINES_H

