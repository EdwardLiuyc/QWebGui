#-------------------------------------------------
#
# Project created by QtCreator 2018-05-25T11:47:46
#
#-------------------------------------------------

QT       += core gui opengl
QT += websockets
CONFIG += C++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QWebGui
TEMPLATE = app
INCLUDEPATH += rapidjson/include

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
    pugixml/pugixml.cpp \
    mapmanagementview.cpp \
    addmapsettingdlg.cpp \
    common_defines.cpp \
    msgbox.cpp

HEADERS  += qwebgui.h \
    robot.h \
    robotmanagementview.h \
    statusmonitorview.h \
    common_defines.h \
    robotselectview.h \
    pugixml/pugiconfig.hpp \
    pugixml/pugixml.hpp \
    rapidjson/include/rapidjson/error/en.h \
    rapidjson/include/rapidjson/error/error.h \
    rapidjson/include/rapidjson/internal/biginteger.h \
    rapidjson/include/rapidjson/internal/diyfp.h \
    rapidjson/include/rapidjson/internal/dtoa.h \
    rapidjson/include/rapidjson/internal/ieee754.h \
    rapidjson/include/rapidjson/internal/itoa.h \
    rapidjson/include/rapidjson/internal/meta.h \
    rapidjson/include/rapidjson/internal/pow10.h \
    rapidjson/include/rapidjson/internal/regex.h \
    rapidjson/include/rapidjson/internal/stack.h \
    rapidjson/include/rapidjson/internal/strfunc.h \
    rapidjson/include/rapidjson/internal/strtod.h \
    rapidjson/include/rapidjson/internal/swap.h \
    rapidjson/include/rapidjson/msinttypes/inttypes.h \
    rapidjson/include/rapidjson/msinttypes/stdint.h \
    rapidjson/include/rapidjson/allocators.h \
    rapidjson/include/rapidjson/document.h \
    rapidjson/include/rapidjson/encodedstream.h \
    rapidjson/include/rapidjson/encodings.h \
    rapidjson/include/rapidjson/filereadstream.h \
    rapidjson/include/rapidjson/filewritestream.h \
    rapidjson/include/rapidjson/fwd.h \
    rapidjson/include/rapidjson/istreamwrapper.h \
    rapidjson/include/rapidjson/memorybuffer.h \
    rapidjson/include/rapidjson/memorystream.h \
    rapidjson/include/rapidjson/ostreamwrapper.h \
    rapidjson/include/rapidjson/pointer.h \
    rapidjson/include/rapidjson/prettywriter.h \
    rapidjson/include/rapidjson/rapidjson.h \
    rapidjson/include/rapidjson/reader.h \
    rapidjson/include/rapidjson/schema.h \
    rapidjson/include/rapidjson/stream.h \
    rapidjson/include/rapidjson/stringbuffer.h \
    rapidjson/include/rapidjson/writer.h \
    mapmanagementview.h \
    addmapsettingdlg.h \
    msgbox.h

RESOURCES += \
    icons.qrc
