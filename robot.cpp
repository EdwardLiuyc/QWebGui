#include "robot.h"
#include "rapidjson/document.h"

Robot::Robot(QObject *parent)
    : QObject( parent )
    , selected_for_connect_( false )
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
    qDebug() << "WebSocket connected to " << url_;
    connected_ = true;
    connect( &socket_, &QWebSocket::textMessageReceived,this, &Robot::slotOnSocketRecieveMsg);
}

void Robot::slotOnSocketDisconnected()
{
    qDebug() << "WebSocket disconnected " << url_;
    connected_ = false;
}

void Robot::slotOnSocketRecieveMsg(QString message)
{
    qDebug() << message;

    parseRecievedMsg( message );
}

void Robot::connectSocket()
{
    qDebug() << "connect to" << url_;
    if( !connected_ )
        socket_.open( url_ );
}

void Robot::disconnectSocket()
{
    if( connected_ )
        socket_.close();
}

int32_t Robot::parseRecievedMsg(QString &msg)
{
    rapidjson::Document doc;
    if( !doc.Parse(msg.toStdString().c_str()).HasParseError() )
    {
        if( doc.HasMember("command") && doc["command"].IsString() )
        {
            const char* cmd = doc["command"].GetString();
            // in general, the command's value is a member of the massage
            if( doc.HasMember(cmd) )
            {
                if( doc[cmd].IsObject() )
                {
                    const rapidjson::Value& object = doc[cmd];
                    if( strstr(cmd, "states") )
                    {
                        if( object.HasMember("x") && object["x"].IsDouble() )
                            state_.x = object["x"].GetDouble();
                        if( object.HasMember("y") && object["y"].IsDouble() )
                            state_.y = object["y"].GetDouble();
                        if( object.HasMember("z") && object["z"].IsDouble() )
                            state_.yaw = object["z"].GetDouble();
                        if( object.HasMember("yaw") && object["yaw"].IsDouble() )
                            state_.yaw = object["yaw"].GetDouble();
                        if( object.HasMember("battery") && object["battery"].IsDouble() )
                            state_.battery = object["battery"].GetDouble();
                        if( object.HasMember("error") && object["error"].IsDouble() )
                            state_.error = (int32_t)( object["error"].GetDouble() + 1e-6 );
                    }
                    else if( strstr(cmd, "gps") )
                    {
                        if( object.HasMember("x") && object["x"].IsDouble() )
                            state_.gps_pos.x = object["x"].GetDouble();
                        if( object.HasMember("y") && object["y"].IsDouble() )
                            state_.gps_pos.y = object["y"].GetDouble();
                        if( object.HasMember("z") && object["z"].IsDouble() )
                            state_.gps_pos.z = object["z"].GetDouble();
                        if( object.HasMember("mode") && object["mode"].IsDouble() )
                            state_.gps_pos.mode = (int32_t)( object["mode"].GetDouble() + 1e-6 );
                    }

                }
                else if ( doc[cmd].IsString() )
                {

                }
            }
            else if( doc.HasMember("value") )
            {
                if( doc["value"].IsString() )
                    qDebug() << cmd << " : " << doc["value"].GetString();
                else if( doc["value"].IsArray() )
                {
                    const rapidjson::Value& array = doc["value"];
                    size_t len = array.Size();
                    QString str = cmd;
                    for( size_t i = 0; i < len; ++i )
                        if( array[i].IsDouble() )
                        {
                            str += QString::number( array[i].GetDouble(), 'd', 3 );
                            str += " ";
                        }
                    qDebug() << str;
                }
            }
        }
        else
        {
            qDebug() << "It is a massage without \"command\", not complete! ";
        }
    }
    else
        return -1;

    return 0;
}
