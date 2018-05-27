#-------------------------------------------------
#
# Project created by QtCreator 2018-05-25T11:47:46
#
#-------------------------------------------------

QT       += core gui
QT += websockets
CONFIG += C++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QWebGui
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += main.cpp\
        qwebgui.cpp \
    robot.cpp \
    robotmanagementview.cpp \
    statusmonitorview.cpp \
    robotselectview.cpp \
    pugixml/pugixml.cpp

HEADERS  += qwebgui.h \
    robot.h \
    robotmanagementview.h \
    statusmonitorview.h \
    common_defines.h \
    robotselectview.h \
    pugixml/pugiconfig.hpp \
    pugixml/pugixml.hpp
