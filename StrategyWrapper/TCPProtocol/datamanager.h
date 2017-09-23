#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <QTcpServer>
#include <QMutex>
#include "tcpprotocol_global.h"
#include <QTimer>
#include <QDateTime>
#include "sockethandler.h"
#include "vistimerthread.h"
#include "mk5client.h"
#include "dbhandler.h"
#include "serializer.h"
#include <QHostAddress>
#include <QDebug>

#define MULTITHREAD false

class TCPPROTOCOLSHARED_EXPORT DataManager : public QObject
{
     Q_OBJECT
public:
    DataManager();
    ~DataManager();
    void newField(quint32 id, quint32 dataSize, QTcpSocket* socket);
    void sendField(quint32 id, QTcpSocket* socket);

signals:
    void newStratData(quint32);
    void newVisData(QByteArray& data);
    void newDBData();

    void newSTSData(quint32);
    void newLTSData(quint32);
    void newWFSData(quint32);
    void newParamData(quint32);

public slots:
    void removeSocket(QTcpSocket*);
    void newConnection();
    void disconnected();
    void newClientType(ClientType type, QTcpSocket* socket);
    void timerCallBack();

private:
    void setAndStartTimer();
    Serializer serializer;
    //DBHandler dbHandler;
    QDateTime time;

    QHash<QTcpSocket*, SocketHandler> socketHash;
    QTcpServer* server;
    MK5Client mk5;

    QMutex socketHashMutex;
    QMutex socketHandlerListMutex;
    QTimer visTimer;
    VisTimerThread visTimerThread;
};

#endif // DATAMANAGER_H
