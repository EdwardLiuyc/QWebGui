#include "robot.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"


#define DEFAULT_HISTORY_COUNT   200
#define MAX_HISTORY_COUNT       20000

Robot::Robot(QObject *parent)
    : QObject( parent )
    , selected_for_connect_( false )
    , connected_( false )
    , history_count_( DEFAULT_HISTORY_COUNT )
{
    connect( &socket_, SIGNAL(connected()), this, SLOT(slotOnSocketConnected()));
    connect( &socket_, SIGNAL(disconnected()), this, SLOT(slotOnSocketDisconnected()));
    connect( &socket_, &QWebSocket::textMessageReceived,this, &Robot::slotOnSocketRecieveMsg);
}

Robot::~Robot()
{
    socket_.close();
}

void Robot::slotOnSocketConnected()
{
    qDebug() << "WebSocket connected to " << url_;
    connected_ = true;

    DisplayMessage msg;
    msg.msg_ = "Robot:" + settings_.name_ + " Connected!";
    msg.level_ = kNormalMsg;
    emit signalRobotRcvNormalMsg( msg );
}

void Robot::slotOnSocketDisconnected()
{
    qDebug() << "WebSocket disconnected " << url_;
    connected_ = false;

    DisplayMessage msg;
    msg.msg_ = "Robot:" + settings_.name_ + " Disconnected!";
    msg.level_ = kErrorMsg;
    emit signalRobotRcvNormalMsg( msg );
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

void Robot::setRecordAllHistory(bool flag)
{
    if(flag)
        history_count_ = MAX_HISTORY_COUNT;
    else
        history_count_ = DEFAULT_HISTORY_COUNT;
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

                        recordState();
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
            else if( strstr(cmd, "alert") && doc.HasMember("value") )
            {
                QString normal_msg;
                if( doc["value"].IsString() )
                    normal_msg = doc["value"].GetString();
                else if( doc["value"].IsArray() )
                {
                    const rapidjson::Value& array = doc["value"];
                    size_t len = array.Size();
                    normal_msg.clear();
                    for( size_t i = 0; i < len; ++i )
                        if( array[i].IsDouble() )
                        {
                            normal_msg += QString::number( array[i].GetDouble(), 'd', 3 );
                            normal_msg += " ";
                        }
                }

                qDebug() << normal_msg;
                DisplayMessage msg;
                msg.msg_ = normal_msg;
                msg.level_ = kNormalMsg;
                emit signalRobotRcvNormalMsg( msg );
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

void Robot::sendCommand(int32_t id, QString cmd, double *array, int32_t size)
{
    if( !connected_ )
    {
        qDebug() << "Not Connected Yet!";
        return;
    }

    rapidjson::StringBuffer string_buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(string_buffer);

    writer.StartObject();

    writer.Key("ID");
    writer.Int(id);

    writer.Key("command");
    writer.String( cmd.toStdString().c_str() );

    writer.Key("value");
    writer.StartArray();
    for( int i = 0; i < size; ++i )
        writer.Double( array[i] );
    writer.EndArray();

    writer.EndObject();

    QString command_str = string_buffer.GetString();
    socket_.sendTextMessage( command_str );
}

void Robot::sendCommand(int32_t id, QString cmd, int32_t value)
{
    if( !connected_ )
    {
        qDebug() << "Not Connected Yet!";
        return;
    }

    rapidjson::StringBuffer string_buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(string_buffer);

    writer.StartObject();

    writer.Key("ID");
    writer.Int(id);

    writer.Key("command");
    writer.String( cmd.toStdString().c_str() );

    writer.Key("value");
    writer.Int( value );

    writer.EndObject();

    QString command_str = string_buffer.GetString();
    socket_.sendTextMessage( command_str );
}

void Robot::sendCommand_AddPoint( double stop_time, AddPointType type )
{
    int32_t id = 1;
    QString cmd = "AddPoint";
    const int32_t value_num = 5;
    double value[value_num];
    value[0] = state_.x;
    value[1] = state_.y;
    value[2] = state_.yaw;
    value[3] = stop_time;
    value[4] = (double)type;

    sendCommand( id,
                 cmd,
                 value,
                 value_num);
}

void Robot::sendCommand_AddPoint(double x, double y, double yaw, double stop_time, AddPointType type)
{
    int32_t id = 1;
    QString cmd = "AddPoint";
    const int32_t value_num = 5;
    double value[value_num];
    value[0] = x;
    value[1] = y;
    value[2] = yaw;
    value[3] = stop_time;
    value[4] = (double)type;

    sendCommand( id,
                 cmd,
                 value,
                 value_num);
}


enum NavigationMode
{
    kSinglePath = 0,
    kLoop = 1,
    kReverse = 2,
    kNavigationModeCount
};

#define OLD_CMD_FORMAT  0
void Robot::sendCommand_SetReverseMode()
{
    int32_t id = 1;
    QString cmd = "NavigationMode";
#if OLD_CMD_FORMAT
    int32_t value = (int32_t)kReverse;
    sendCommand( id, cmd, value );
#else
    const int32_t value_num = 1;
    double value[value_num];
    value[0] = (double)kReverse;

    sendCommand( id, cmd, value, value_num);
#endif

}

void Robot::sendCommand_SetLoopMode()
{
    int32_t id = 1;
    QString cmd = "NavigationMode";
#if OLD_CMD_FORMAT
    int32_t value = (int32_t)kLoop;
    sendCommand( id, cmd, value );
#else
    const int32_t value_num = 1;
    double value[value_num];
    value[0] = (double)kLoop;

    sendCommand( id, cmd, value, value_num);
#endif
}

void Robot::sendCommand_Run()
{
    int32_t id = 1;
    QString cmd = "onStart";
#if OLD_CMD_FORMAT
    int32_t value = 1;
    sendCommand( id, cmd, value );
#else
    const int32_t value_num = 1;
    double value[value_num];
    value[0] = 1.0;

    sendCommand( id, cmd, value, value_num);
#endif
}

void Robot::sendCommand_Halt()
{
    int32_t id = 1;
    QString cmd = "onStart";
#if OLD_CMD_FORMAT
    int32_t value = 0;
    sendCommand( id, cmd, value );
#else
    const int32_t value_num = 1;
    double value[value_num];
    value[0] = 0.0;

    sendCommand( id, cmd, value, value_num);
#endif
}

void Robot::sendCommand_SetRunningMode(RobotRunningMode mode)
{
    int32_t id = 1;
    QString cmd = "ModeSwitch";
#if OLD_CMD_FORMAT
    sendCommand( id, cmd, (int32_t)mode );
#else
    const int32_t value_num = 1;
    double value[value_num];
    value[0] = (double)mode;

    sendCommand( id, cmd, value, value_num);
#endif
}

void Robot::sendCommand_ManualRun(double strength, double angle)
{
    int32_t id = 1;
    QString cmd = "ManualMove";
    const int32_t value_num = 2;
    double value[value_num] = { strength, angle };

    sendCommand( id, cmd, value, value_num);
}

int32_t Robot::recordState()
{
    state_history_.push_back( state_ );
    while( (int32_t)state_history_.size() > history_count_ )
        state_history_.pop_front();

    return state_history_.size();
}
