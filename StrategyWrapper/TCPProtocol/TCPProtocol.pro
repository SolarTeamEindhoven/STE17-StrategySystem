#-------------------------------------------------
#
# Project created by QtCreator 2017-08-24T10:56:14
#
#-------------------------------------------------

QT       += core
QT       += network
QT       += sql
QT       -= gui

TARGET = TCPProtocol
TEMPLATE = lib

DEFINES += TCPPROTOCOL_LIBRARY

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
    datamanager.cpp \
    serializer.cpp \
    headerhandler.cpp \
    writehandler.cpp \
    dbhandler.cpp

HEADERS += \
        tcpprotocol_global.h \  
    datamanager.h \
    serializer.h \
    headerhandler.h \
    sockethandler.h \
    writehandler.h \
    dbhandler.h

STE_BUILD_PATH_PREFIX = $$relative_path($$OUT_PWD)

INCLUDEPATH+="C:\Program Files\MySQL\include"

LIBS+="C:\Program Files\MySQL\MySQL Server 5.7\lib\libmysql.lib"

unix {
    target.path = /usr/lib
    INSTALLS += target
}
win32{
    CONFIG(release, debug|release): target.path += $$PWD/$$STE_BUILD_PATH_PREFIX/../DataManager/release/
    CONFIG(debug, debug|release): target.path += $$PWD/$$STE_BUILD_PATH_PREFIX/../DataManager/debug
    INSTALLS += target
}
