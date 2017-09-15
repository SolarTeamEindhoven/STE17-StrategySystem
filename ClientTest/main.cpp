#include <QCoreApplication>
#include "tcpclient.h"
#include <qDebug>


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    TcpClient client(1);

    client.identify(1);

    TcpClient client2(2);
    client2.identify(1);
    client2.writeLargeData();
    TcpClient client3(3);
    client3.identify(2);
    client.setTimer();
    return a.exec();
}
