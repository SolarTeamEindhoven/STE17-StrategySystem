#include "tcpconnection.h"

TCPConnection::TCPConnection()
{
    socket = new QTcpSocket();
    this->start();
}

void TCPConnection::run() {
    qDebug() << "Running";
    exec();
}
