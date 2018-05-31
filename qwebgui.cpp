#include "qwebgui.h"
#include <QDebug>
#include <QStyleFactory>
#include "common_defines.h"
#include "pugixml/pugixml.hpp"
#include "pugixml/pugiconfig.hpp"

QWebGui::QWebGui(QWidget *parent)
    : QMainWindow(parent)
    , robot_manage_view_( NULL )
    , status_monitor_view_( NULL )
    , map_manage_view_( NULL )
{
    const QString btn_texts[kMainBtnCount] =
    {
        "Robot\nManagement", "Map\nManagement", "Path\nManagement", "Status\nMonitor"
    };
    for( int i = 0; i < kMainBtnCount; ++i )
    {
        main_btns_[i] = new QPushButton(this);
        main_btns_[i]->setText(btn_texts[i]);
        main_btns_[i]->setMaximumSize( 300, 200 );
        main_btns_[i]->setFont( SYSTEM_UI_FONT_18_BOLD );
        main_btns_[i]->setFocusPolicy(Qt::NoFocus);
    }
    QObject::connect( main_btns_[kRobotManage], SIGNAL(clicked()), this, SLOT(slotOnRobotManageClicked()));
    QObject::connect( main_btns_[kStatusMonitor], SIGNAL(clicked()), this, SLOT(slotOnStatusMonitorClicked()));
    QObject::connect( main_btns_[kMapManage], SIGNAL(clicked()), this, SLOT(slotOnMapManageClicked()));
    QObject::connect( main_btns_[kPathManage], SIGNAL(clicked()), this, SLOT(slotOnPathManageClicked()));

    resize( 1366, 768 );
    setMinimumSize( 1280, 720 );

    initWithSettingFile( DEFAULT_SETTING_FILE );
}

#define REMAIN_HEIFHT_FOR_VIEW 30
void QWebGui::resizeEvent(QResizeEvent *event)
{
    int window_wdt = this->width();
    int window_hgt = this->height();

    // MAIN BUTTONS
    int main_btn_gap = 25;
    int main_btn_hgt = 150;
    int main_btn_wdt = (window_wdt - main_btn_gap ) / 2;
    if( main_btn_wdt > 300 )
        main_btn_wdt = 300;
    int main_btn_left = ( window_wdt - main_btn_gap  - main_btn_wdt * 2 ) / 2;
    int main_btn_top = ( window_hgt - main_btn_hgt * 2 - main_btn_gap ) / 2;
    main_btn_top = ( main_btn_top >= 0 ? main_btn_top : 0 );
    for( int i = 0; i < kMainBtnCount; ++i )
    {
        main_btns_[i]->setGeometry( main_btn_left + (i % 2) * (main_btn_wdt+main_btn_gap),
                                    main_btn_top + ( main_btn_hgt+ main_btn_gap) * (i/2),
                                    main_btn_wdt,
                                    main_btn_hgt);
    }

    // ROBOT MANAGE VIEW
    if( robot_manage_view_ )
        robot_manage_view_->setGeometry( 0, REMAIN_HEIFHT_FOR_VIEW, window_wdt, window_hgt-REMAIN_HEIFHT_FOR_VIEW*2 );
    if( status_monitor_view_  )
        status_monitor_view_->setGeometry( 0, 0, window_wdt, window_hgt );
    if( map_manage_view_  )
        map_manage_view_->setGeometry( 0, REMAIN_HEIFHT_FOR_VIEW, window_wdt, window_hgt-REMAIN_HEIFHT_FOR_VIEW*2 );


    QMainWindow::resizeEvent( event );
}

QWebGui::~QWebGui()
{

}


// ***** Robot Management *****
void QWebGui::slotOnRobotManageClicked()
{
    if( !robot_manage_view_ )
    {
        robot_manage_view_ = new RobotManagementView( &robot_setting_list_, this );
        robot_manage_view_->setGeometry( 0, REMAIN_HEIFHT_FOR_VIEW, this->width(), this->height()-REMAIN_HEIFHT_FOR_VIEW*2 );

        connect( robot_manage_view_, SIGNAL(signalReturn()), this, SLOT(slotOnRobotManageReturned()));
        connect( robot_manage_view_, SIGNAL(signalSaveSetting()), this, SLOT(slotOnSaveSetting()));
    }

    robot_manage_view_->show();
    for( int i = 0; i < kMainBtnCount; ++i )
        main_btns_[i]->setVisible(false);
}

void QWebGui::slotOnRobotManageReturned()
{
    for( int i = 0; i < kMainBtnCount; ++i )
        main_btns_[i]->setVisible(true);

}


// ***** Map Management *****
void QWebGui::slotOnMapManageClicked()
{
    if( !map_manage_view_ )
    {
        map_manage_view_ = new MapManagementView( &map_setting_list_, this );
        map_manage_view_->setGeometry( 0, REMAIN_HEIFHT_FOR_VIEW, this->width(), this->height()-REMAIN_HEIFHT_FOR_VIEW*2 );
        connect( map_manage_view_, SIGNAL(signalReturn()), this, SLOT(slotOnMapManageReturned()));
        connect( map_manage_view_, SIGNAL(signalSaveSetting()), this, SLOT(slotOnSaveSetting()));
    }
    map_manage_view_->show();
    for( int i = 0; i < kMainBtnCount; ++i )
        main_btns_[i]->setVisible(false);
}

void QWebGui::slotOnMapManageReturned()
{
    for( int i = 0; i < kMainBtnCount; ++i )
        main_btns_[i]->setVisible(true);
}


// ***** State Monitor *****
void QWebGui::slotOnStatusMonitorReturned()
{
    for( int i = 0; i < kMainBtnCount; ++i )
        main_btns_[i]->setVisible(true);

    this->setWindowTitle("QWebGui");
}

void QWebGui::slotOnStatusMonitorClicked()
{
    if( !status_monitor_view_ )
    {
        status_monitor_view_ = new StatusMonitorView( &robots_, &map_setting_list_, this);
        status_monitor_view_->setGeometry( 0, 0, this->width(), this->height() );

        connect( status_monitor_view_, SIGNAL(signalReturn()), this, SLOT(slotOnStatusMonitorReturned()));
    }

    updateRobotListFromSettings();
    status_monitor_view_->setMode( MonitorMode::kStatusMonitorMode );
    status_monitor_view_->show();
    for( int i = 0; i < kMainBtnCount; ++i )
        main_btns_[i]->setVisible(false);

    this->setWindowTitle("Status Monitor");
}


// ***** Path Management *****
void QWebGui::slotOnPathManageClicked()
{
    if( !status_monitor_view_ )
    {
        status_monitor_view_ = new StatusMonitorView( &robots_, &map_setting_list_, this, MonitorMode::kPathManageMode );
        status_monitor_view_->setGeometry( 0, 0, this->width(), this->height() );

        connect( status_monitor_view_, SIGNAL(signalReturn()), this, SLOT(slotOnStatusMonitorReturned()));
    }

    updateRobotListFromSettings();
    status_monitor_view_->setMode( MonitorMode::kPathManageMode );
    status_monitor_view_->show();
    for( int i = 0; i < kMainBtnCount; ++i )
        main_btns_[i]->setVisible(false);

    this->setWindowTitle("Path Management");
}

void QWebGui::slotOnPathManageReturned()
{
    for( int i = 0; i < kMainBtnCount; ++i )
        main_btns_[i]->setVisible(true);

    this->setWindowTitle("QWebGui");
}


void QWebGui::updateRobotListFromSettings()
{
    int32_t setting_size = robot_setting_list_.size();
    int32_t robot_size = robots_.size();

    if( setting_size == 0 )
    {
        robots_.clear();
        return;
    }

    if( robot_size < setting_size )
        for(int i = 0; i < setting_size-robot_size; ++i )
            robots_.emplace_back( this );
    else if( robot_size > setting_size )
        for( int i = 0; i < robot_size-setting_size; ++i )
            robots_.pop_back();

    if( robot_setting_list_.size() != robots_.size() )
    {
        qDebug() << "Error in "<< __FUNCTION__;
    }

    auto setting = robot_setting_list_.begin();
    for( Robot& r : robots_ )
    {
        r.settings_ = *setting;
        qDebug() << setting->DebugString();
        QString url_str = "ws://" + setting->ip_ + ":" + QString::number(setting->port_);
        r.setUrl( QUrl(url_str) );
        setting++;
    }

}

int32_t QWebGui::initWithSettingFile(const char *filename)
{
    if(!filename)
        return -1;

    pugi::xml_document doc;
    if( !doc.load_file( filename ) ) // 0代表 load 成功，没有错误
    {
        qDebug() << "Failed in load xml file :" << filename;
        return -1;
    }

    pugi::xml_node webgui_node = doc.child( "BONGOS_QWEBGUI" );
    if( webgui_node.empty() )
    {
        qDebug() << "Node empty!";
        return -1;
    }

    for( pugi::xml_node setting_node = webgui_node.first_child();
         setting_node;
         setting_node = setting_node.next_sibling() )
    {
        if( strstr(setting_node.name(), "robot_setting" ) )
        {
            RobotSettings setting;
            setting.name_ = setting_node.attribute("name").as_string();
            setting.ip_ = setting_node.attribute("ip").as_string();
            setting.port_ = setting_node.attribute("port").as_int();
            robot_setting_list_.push_back(setting);
        }
        else if( strstr(setting_node.name(), "map_setting" ) )
        {
            MapSetting setting;
            setting.name_ = setting_node.attribute("name").as_string();
            setting.image_file_name_ = setting_node.attribute("image_file").as_string();
            setting.image_info_file_name_ = setting_node.attribute("image_config_file").as_string();
            map_setting_list_.push_back( setting );
        }
    }

    return 0;
}

void QWebGui::slotOnSaveSetting()
{
    saveSettingFile( DEFAULT_SETTING_FILE );
}

int32_t QWebGui::saveSettingFile(const char *filename)
{
    if(!filename)
        return -1;

    pugi::xml_document doc;
    pugi::xml_node pre = doc.prepend_child(pugi::node_declaration);
    pre.append_attribute("version") = "1.0";
    pre.append_attribute("encoding") = "utf-8";

    pugi::xml_node root_node = doc.append_child("BONGOS_QWEBGUI");
    for( RobotSettings& setting : robot_setting_list_ )
    {
        pugi::xml_node setting_node = root_node.append_child("robot_setting");
        setting_node.append_attribute("name").set_value( setting.name_.toStdString().c_str() );
        setting_node.append_attribute("ip").set_value( setting.ip_.toStdString().c_str());
        setting_node.append_attribute("port").set_value( setting.port_ );
    }

    for( MapSetting& setting : map_setting_list_ )
    {
        pugi::xml_node setting_node = root_node.append_child("map_setting");
        setting_node.append_attribute("name").set_value( setting.name_.toStdString().c_str() );
        setting_node.append_attribute("image_file").set_value( setting.image_file_name_.toStdString().c_str());
        setting_node.append_attribute("image_config_file").set_value( setting.image_info_file_name_.toStdString().c_str() );
    }

    doc.save_file( filename, "\t", 1U, pugi::encoding_utf8);
    return 0;
}
