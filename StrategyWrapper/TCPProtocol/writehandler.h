#ifndef WRITEHANDLER_H
#define WRITEHANDLER_H

#include <QTcpSocket>
#include <QTimer>
#include <QtEndian>
#include "readhandler.h"

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
    QTimer flushTimer;

signals:
    void sendVisDataSignal();
    void sendRowsInternalSignal(quint32 id);

public slots:
    void sendRowsRequest(quint32 id);
    void addVisData(QByteArray& data);
    void flush();

private slots:
    void sendVisDataSlot();
    void sendRows(quint32 id);

private:
    QMutex bufferMutex;
    QByteArray visLinesBuffer;
    int bufferLines;
    QMutex writeMutex;
};

#endif // WRITEHANDLER_H
