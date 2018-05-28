#ifndef ROBOT_H
#define ROBOT_H

#include <QObject>
#include <QWebSocket>
#include <QUrl>
#include "common_defines.h"

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

    bool selected_for_connect_;
    bool connected_;
    RobotSettings settings_;
    RobotState state_;


signals:

public slots:
    void slotOnSocketConnected();
    void slotOnSocketDisconnected();
    void slotOnSocketRecieveMsg(QString message);

private:
    QWebSocket      socket_;
    QUrl            url_;

    int32_t parseRecievedMsg(QString& msg);
};

#endif // ROBOT_H
