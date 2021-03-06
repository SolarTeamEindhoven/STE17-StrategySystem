#-------------------------------------------------
#
# Project created by QtCreator 2017-08-09T15:19:17
#
#-------------------------------------------------

QT       += core gui
QT       += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DataManager
CONFIG   += console C++11
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


SOURCES += \
        main.cpp \
    client.cpp \
    server.cpp \
    tcpconnection.cpp

HEADERS += \
    client.h \
    server.h \
    tcpconnection.h \
    tcpprotocol.h

STE_BUILD_PATH_PREFIX = $$relative_path($$OUT_PWD)

CONFIG += link_pkgconfig
# PKGCONFIG += libgps

unix {
    LIBS += \
#        -L$$PWD/$$STE_BUILD_PATH_PREFIX/../QSTECANMessage/ -lQSTECANMessage \
#        -L$$PWD/$$STE_BUILD_PATH_PREFIX/../StreamingLibrary -lStreamingLibrary
}
win32 {
    LIBS += -lwsock32
#    CONFIG(release, debug|release): LIBS+=$$PWD/$$STE_BUILD_PATH_PREFIX/../QSTECANMessage/release/QSTECANMessage.dll \
#                                          $$PWD/$$STE_BUILD_PATH_PREFIX/../StreamingLibrary/release/StreamingLibrary.dll
#    CONFIG(debug, debug|release): LIBS+=$$PWD/$$STE_BUILD_PATH_PREFIX/../QSTECANMessage/debug/QSTECANMessage.dll \
#                                         $$PWD/$$STE_BUILD_PATH_PREFIX/../StreamingLibrary/debug/StreamingLibrary.dll

}
INCLUDEPATH += \
#        $$PWD/../QSTECANMessage/include \
#        $$PWD/../StreamingLibrary/include

QMAKE_CXXFLAGS -= -O
QMAKE_CXXFLAGS -= -O1
QMAKE_CXXFLAGS -= -O2

QMAKE_CXXFLAGS_DEBUG += -Og
QMAKE_CXXFLAGS_RELEASE += -O3
QMAKE_LFLAGS_RELEASE += -flto
