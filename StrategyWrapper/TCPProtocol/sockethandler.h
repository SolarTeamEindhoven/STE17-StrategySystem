#ifndef SOCKETHANDLER_H
#define SOCKETHANDLER_H

#include <QtCore>
#include "headerhandler.h"
#include "writehandler.h"
#include "tcpprotocol_global.h"

struct SocketHandler : public QObject {
public:
    SocketHandler(QTcpSocket* socket, DataManager* parent)
        : writeHandler(socket, parent, this), headerHandler(socket, parent, this), socket(socket), dataManager(parent)
    {}

    SocketHandler(const SocketHandler& copy)
        : QObject(copy.parent()), writeHandler(copy.writeHandler), headerHandler(copy.headerHandler), socket(copy.socket), dataManager(copy.dataManager) {}

    SocketHandler& operator= (const SocketHandler& handler) {
        this->socket = handler.socket;
        this->dataManager = handler.dataManager;
        this->headerHandler = handler.headerHandler;
        this->writeHandler = handler.writeHandler;
        return *this;
    }

    //initialize connects after all those copies etc
    void initializeConnects() {
        writeHandler.initializeConnects();
        headerHandler.initializeConnects();
    }

    WriteHandler writeHandler;
    HeaderHandler headerHandler;
    QTcpSocket* socket;
    QMutex mutex;
    DataManager* dataManager;
};

#endif // SOCKETHANDLER_H
