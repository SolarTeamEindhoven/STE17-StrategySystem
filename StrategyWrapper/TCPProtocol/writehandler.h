#ifndef WRITEHANDLER_H
#define WRITEHANDLER_H

#include <QTcpSocket>
#include <QTimer>
#include <QtEndian>
#include "headerhandler.h"

class TCPPROTOCOLSHARED_EXPORT WriteHandler : public QObject
{
    Q_OBJECT
public:
    WriteHandler(QTcpSocket* socket, DataManager* dataManager, SocketHandler* socketHandler);
    WriteHandler(const WriteHandler& copy) : QObject(copy.parent()), socket(copy.socket), dataManager(copy.dataManager), socketHandler(copy.socketHandler) {}
    WriteHandler& operator=(const WriteHandler& other) {
        this->socket = other.socket;
        this->dataManager = other.dataManager;
        this->socketHandler = other.socketHandler;
        return *this;
    }

    void initializeConnects();

    QTcpSocket* socket;
    DataManager* dataManager;
    SocketHandler* socketHandler;

public slots:
    void sendRows(quint32 id, bool multipleLines);
    void sendVisData(QByteArray& data);
    void flush();
private:
    QByteArray collectedVisRows;
    QTimer flushTimer;
    QMutex mutex;
};

#endif // WRITEHANDLER_H
