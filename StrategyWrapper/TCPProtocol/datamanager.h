#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <QTcpServer>
#include <QMutex>
#include "tcpprotocol_global.h"
#include <QTimer>
#include <QDateTime>
#include "sockethandler.h"
#include "dbhandler.h"
#include "serializer.h"
#include <QHostAddress>
#include <QDebug>

class TCPPROTOCOLSHARED_EXPORT DataManager : public QObject
{
     Q_OBJECT
public:
    DataManager();
    void newField(quint32 id, quint32 dataSize, QTcpSocket* socket, bool multipleLines);
    void sendField(quint32 id, QTcpSocket* socket, bool multipleLines);

signals:
    void newStratData(quint32, bool);
    void newVisData(QByteArray& data);
    void newDBData();

    void newSTSData(quint32, bool);
    void newLTSData(quint32, bool);
    void newWFSData(quint32, bool);
    void newParamData(quint32, bool);

public slots:
    void newConnection();
    void disconnected();
    void newClientType(ClientType type);
    void timerCallBack();

private:
    void setAndStartTimer();
    Serializer serializer;
    DBHandler dbHandler;
    QDateTime time;

    QHash<QTcpSocket*, SocketHandler> socketHash;
    QTcpServer* server;

    QMutex socketHashMutex;
    QMutex socketHandlerListMutex;
    QTimer timer;
};

#endif // DATAMANAGER_H
