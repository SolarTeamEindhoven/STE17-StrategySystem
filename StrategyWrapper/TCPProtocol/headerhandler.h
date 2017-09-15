#ifndef READHANDLER_H
#define READHANDLER_H

#include <QtCore>
#include <QTcpSocket>
#include <QTEndian>
#include "tcpprotocol_global.h"
#include "QThread"

enum ClientType {
    mk5,
    visualizer,
    strategy,
    weather,
    unclear
};

class DataManager;
struct SocketHandler;

class TCPPROTOCOLSHARED_EXPORT HeaderHandler : public QThread
{
     Q_OBJECT
public:
    HeaderHandler(QTcpSocket* socket, DataManager* dataManager, SocketHandler* socketHandler);
    HeaderHandler(const HeaderHandler& copy);
    HeaderHandler& operator=(const HeaderHandler& other) {
        this->clientType = other.clientType;
        this->socket = other.socket;
        this->dataManager = other.dataManager;
        this->socketHandler = other.socketHandler;
        return *this;
    }
    virtual ~HeaderHandler();
    void initializeConnects();

    ClientType clientType;
    QTcpSocket* socket;
    DataManager* dataManager;
    SocketHandler* socketHandler;
signals:
    void newClientType(ClientType type);

public slots:
    void readyRead();

private:
    void run() override;
    enum HHState {readType, readSize, readData, readNewClient};
    HHState state;
    void readNextType();
    void readNextSize();
    void readNextData();
    void readNextNewClient();

    quint32 id;
    quint32 size;
    bool multipleLines;

};

#endif // READHANDLER_H
