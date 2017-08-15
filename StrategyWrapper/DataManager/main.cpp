#include "client.h"
#include "server.h"
#include <QApplication>
#include <QThread>


//http://doc.qt.io/qt-4.8/qtendian.html TODO
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QThread serverThread;
    Server server;
    server.moveToThread(&serverThread);

    Client client;
    qDebug() << "Client connecting status " << client.connectToHost("localhost");
    server.test();
    for (qint32 i = 0; i < 1000; i++) {
        qDebug() << "Client sending " << i;
        client.writeData(intToArray(i));
        QThread::msleep(1000);
    }

    return a.exec();
}
