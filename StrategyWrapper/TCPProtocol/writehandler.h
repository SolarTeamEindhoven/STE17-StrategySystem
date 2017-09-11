#ifndef WRITEHANDLER_H
#define WRITEHANDLER_H

#include <QTcpSocket>
#include <QTimer>
#include <QtEndian>
#include "headerhandler.h"

class TCPPROTOCOLSHARED_EXPORT WriteHandler : public QObject
{
public:
    WriteHandler(QTcpSocket* socket, DataManager* dataManager, SocketHandler* parent);
    WriteHandler(const WriteHandler& handler) : QObject(this), socket(handler.socket), dataManager(handler.dataManager), parent(handler.parent) {}
    WriteHandler& operator=(const WriteHandler& other) {
        this->socket = other.socket;
        this->dataManager = other.dataManager;
        this->parent = other.parent;
        return *this;
    }

    void initializeConnects();

    QTcpSocket* socket;
    DataManager* dataManager;
    SocketHandler* parent;

public slots:
    void sendRows(quint32 id, quint32 size, QByteArray& data);
    void sendRow(quint32 id, QByteArray& data);
    void flush();
private:
    QTimer flushTimer;
};

#endif // WRITEHANDLER_H
