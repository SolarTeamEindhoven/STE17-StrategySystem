#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <QTcpServer>
#include <QMutex>
#include "tcpprotocol_global.h"
#include <QTimer>
#include <QDateTime>
#include "sockethandler.h"
#include "serializer.h"
#include <QHostAddress>
#include <QDebug>

#define DATAPERIOD 100 //in ms

class TCPPROTOCOLSHARED_EXPORT DataManager : public QObject
{
     Q_OBJECT
public:
    DataManager();
    void newField(quint32 id, quint32 dataSize, QTcpSocket* socket, bool multipleLines);

signals:
    void newStratData(quint32, QByteArray&);
    void newVisData(quint32, QByteArray&);
    void newDBData();

    void newSTSData(quint32, quint32, QByteArray&);
    void newLTSData(quint32, QByteArray&);
    void newWFSData(quint32, quint32, QByteArray&);
    void newParamData(quint32, QByteArray&);

public slots:
    void newConnection();
    void disconnected();
    void newClientType(ClientType type);
    void timerCallBack();

private:
    void setAndStartTimer();
    Serializer serializer;
    QDateTime time;

    QHash<QTcpSocket*, SocketHandler> socketHash;
    QTcpServer* server;

    QMutex socketHashMutex;
    QMutex socketHandlerListMutex;
    QTimer timer;
};

#endif // DATAMANAGER_H
