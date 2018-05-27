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

    inline void setUrl( QUrl url ) { url_ = url; }
    void connectSocket();

    bool selected_;
    bool connected_;
    RobotSettings settings_;

signals:
    void signalConnectStatusChanged();

public slots:
    void slotOnSocketConnected();
    void slotOnSocketDisconnected();
    void slotOnSocketRecieveMsg(QString message);

private:
    QWebSocket      socket_;
    QUrl            url_;
};

#endif // ROBOT_H
