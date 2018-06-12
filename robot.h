#ifndef ROBOT_H
#define ROBOT_H

#include <QObject>
#include <QWebSocket>
#include <QUrl>
#include "common_defines.h"


class Robot : public QObject
{
    Q_OBJECT

public:
    explicit Robot(QObject *parent = 0);
    ~Robot();

    // settings
    inline void setUrl( QUrl url ) { url_ = url; }
    void setRecordAllHistory( bool flag );
    void connectSocket();
    void disconnectSocket();

    // send commands
    void sendCommand_AddPoint(double stop_time = 0., AddPointType type = kNormal);
    void sendCommand_AddPoint(double x, double y, double yaw, double stop_time = 0., AddPointType type = kNormal);
    void sendCommand_SetReverseMode();
    void sendCommand_SetLoopMode();
    void sendCommand_Run();
    void sendCommand_Halt();
    void sendCommand_SetRunningMode( RobotRunningMode mode );
    void sendCommand_ManualRun( double strength, double angle );
    void sendCommand_AddObstacleArea( const QList<QPointF>& polygon_in_map );
    void sendCommand_CommandFinish(QString cmd_name, double val );

    bool selected_for_connect_;
    bool connected_;
    RobotSettings settings_;
    RobotState state_;

    // gets
    inline std::list<RobotState>* getHistrory(){ return &state_history_; }

protected:
    void sendCommand( int32_t id, QString cmd, double* array, int32_t size );
    void sendCommand( int32_t id, QString cmd, int32_t value );

    int32_t recordState();

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

    std::list<RobotState> state_history_;
    std::list<RobotState> state_future_;
    int32_t history_count_;
    int32_t future_count_;

    int32_t parseRecievedMsg(QString& msg);
};

#endif // ROBOT_H
