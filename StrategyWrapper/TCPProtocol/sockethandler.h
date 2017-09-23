#ifndef SOCKETHANDLER_H
#define SOCKETHANDLER_H

#include <QtCore>
#include "readhandler.h"
#include "writehandler.h"
#include "tcpprotocol_global.h"

struct SocketHandler : public QObject {
public:
    SocketHandler(QTcpSocket* socket, DataManager* parent)
        : writeHandler(socket, parent, this), readHandler(socket, parent, this), socket(socket), dataManager(parent)
    {}

    SocketHandler(const SocketHandler& copy)
        : QObject(copy.parent()), writeHandler(copy.writeHandler), readHandler(copy.readHandler), socket(copy.socket), dataManager(copy.dataManager) {}

    SocketHandler& operator= (const SocketHandler& handler) {
        this->socket = handler.socket;
        this->dataManager = handler.dataManager;
        this->readHandler = handler.readHandler;
        this->writeHandler = handler.writeHandler;
        return *this;
    }
    ~SocketHandler() {
    }

    //initialize connects after all those copies etc
    void initializeConnects() {
        writeHandler.initializeConnects();
        readHandler.initializeConnects();
    }

    WriteHandler writeHandler;
    ReadHandler readHandler;
    QTcpSocket* socket;
    DataManager* dataManager;
};

#endif // SOCKETHANDLER_H
