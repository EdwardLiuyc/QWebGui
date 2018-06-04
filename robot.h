#ifndef ROBOT_H
#define ROBOT_H

#include <QObject>
#include <QWebSocket>
#include <QUrl>
#include "common_defines.h"
#include <queue>

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


class Robot : public QObject
{
    Q_OBJECT

public:
    explicit Robot(QObject *parent = 0);
    ~Robot();

    inline void setUrl( QUrl url ) { url_ = url; }
    void connectSocket();
    void disconnectSocket();

    // commands
    void sendCommand_AddPoint(double stop_time = 0., AddPointType type = kNormal);
    void sendCommand_AddPoint(double x, double y, double yaw, double stop_time = 0., AddPointType type = kNormal);
    void sendCommand_SetReverseMode();
    void sendCommand_SetLoopMode();
    void sendCommand_Run();
    void sendCommand_Halt();
    void sendCommand_SetRunningMode( RobotRunningMode mode );
    void sendCommand_ManualRun( double strength, double angle );

    bool selected_for_connect_;
    bool connected_;
    RobotSettings settings_;
    RobotState state_;

protected:
    void sendCommand( int32_t id, QString cmd, double* array, int32_t size );
    void sendCommand( int32_t id, QString cmd, int32_t value );

signals:
    void signalRobotConnected();
    void signalRobotDisconnected();
    void signalRobotRcvNormalMsg( DisplayMessage& msg );

public slots:
    void slotOnSocketConnected();
    void slotOnSocketDisconnected();
    void slotOnSocketRecieveMsg(QString message);

private:
    QWebSocket      socket_;
    QUrl            url_;

    std::queue<RobotState> state_history_;

    int32_t parseRecievedMsg(QString& msg);
};

#endif // ROBOT_H
