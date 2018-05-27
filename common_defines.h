#ifndef COMMON_DEFINES_H
#define COMMON_DEFINES_H

#include <QFont>
#include <QPushButton>
#include <list>

#define SYSTEM_UI_FONT_12   QFont("system-ui",12,QFont::Light)
#define SYSTEM_UI_FONT_14   QFont("system-ui",14,QFont::Light)
#define SYSTEM_UI_FONT_16   QFont("system-ui",16,QFont::Light)
#define SYSTEM_UI_FONT_18   QFont("system-ui",18,QFont::Light)

#define SYSTEM_UI_FONT_12_BOLD   QFont("system-ui",12,QFont::Bold)
#define SYSTEM_UI_FONT_14_BOLD   QFont("system-ui",14,QFont::Bold)
#define SYSTEM_UI_FONT_16_BOLD   QFont("system-ui",16,QFont::Bold)
#define SYSTEM_UI_FONT_18_BOLD   QFont("system-ui",18,QFont::Bold)

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

#ifdef Q_OS_LINUX
// create a setting file in current directory
#define DEFAULT_SETTING_FILE "robot_settings.xml"
#else
#define DEFAULT_SETTING_FILE "C:\\robot_settings.xml"
#endif

#endif // COMMON_DEFINES_H

