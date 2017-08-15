#include "server.h"

static inline qint32 ArrayToInt(QByteArray source);

Server::Server(QObject *parent) : QObject(parent)
{
    tcpServer = new QTcpServer(this);
//    qDebug() << "Server: signal connection" << connect(server, SIGNAL(newConnection()), this, SLOT(newConnection()));
    qDebug() << "Server: signal connection" << connect(tcpServer, &QTcpServer::newConnection, this, &Server::newConnection);
    qDebug() << "Server: listening:" << tcpServer->listen(QHostAddress::Any, 1024);
}

void Server::newConnection()
{
    qDebug() << "Server: Connection attempt reqcognized";
    while (tcpServer->hasPendingConnections())
    {

        QTcpSocket *socket = tcpServer->nextPendingConnection();
        connect(socket, SIGNAL(readyRead()), SLOT(readyRead()));
        connect(socket, SIGNAL(disconnected()), SLOT(disconnected()));
        QByteArray *buffer = new QByteArray();
        qint32 *s = new qint32(0);
        buffers.insert(socket, buffer);
        sizes.insert(socket, s);
        qDebug() << "Server: Connection made";
    }
}

void Server::test() {
    qDebug() << "Has pending connections " << tcpServer->hasPendingConnections();
}

void Server::readyRead()
{
    qDebug() << "Server: read data";
}

void Server::disconnected()
{
    QTcpSocket *socket = static_cast<QTcpSocket*>(sender());
    QByteArray *buffer = buffers.value(socket);
    qint32 *s = sizes.value(socket);
    socket->deleteLater();
    delete buffer;
    delete s;
}
