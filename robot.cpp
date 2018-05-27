#include "robot.h"

Robot::Robot(QObject *parent)
    : QObject( parent )
    , selected_( false )
    , connected_( false )
{
    connect( &socket_, SIGNAL(connected()), this, SLOT(slotOnSocketConnected()));
    connect( &socket_, SIGNAL(disconnected()), this, SLOT(slotOnSocketDisconnected()));
}

Robot::~Robot()
{
    socket_.close();
}

void Robot::slotOnSocketConnected()
{
    qDebug() << "WebSocket connected";
    connected_ = true;
    connect( &socket_, &QWebSocket::textMessageReceived,this, &Robot::slotOnSocketRecieveMsg);
}

void Robot::slotOnSocketDisconnected()
{
    qDebug() << "WebSocket disconnected";
    connected_ = false;
}

void Robot::slotOnSocketRecieveMsg(QString message)
{
    (void)message;
}

void Robot::connectSocket()
{
    qDebug() << "connect to" << url_;
    socket_.open( url_ );
}
