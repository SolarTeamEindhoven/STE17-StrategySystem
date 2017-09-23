#include "mk5client.h"

MK5Client::MK5Client(DataManager* dm) : socket(), handler(&socket, dm)
{
    qDebug() << "Trying to connect to mk5";
    socket.moveToThread(this); //don't know whether this is needed
    handler.moveToThread(this);
    handler.writeHandler.moveToThread(this);
    handler.writeHandler.flushTimer.moveToThread(this);
    handler.readHandler.moveToThread(this);
    start();
    setPriority(QThread::TimeCriticalPriority);
    qDebug() << "Mk5 found";
}

void MK5Client::disconnectSocket() {
    socket.disconnect();
    handler.writeHandler.disconnect();
    handler.readHandler.disconnect();
}

void MK5Client::run() {
    QHostAddress address("192.168.1.104");
    socket.connectToHost(address,4232);
    socket.waitForConnected();
//    connect(&socket, SIGNAL(disconnected()), this, SLOT(disconnectSocket()));
    QObject::connect(&socket, &QTcpSocket::disconnected, this, &MK5Client::disconnectSocket);
    handler.initializeConnects();
    qDebug() << "Thread initialized, starting execution";
    exec();
}
