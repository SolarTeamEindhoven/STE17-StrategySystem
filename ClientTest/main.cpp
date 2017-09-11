#include <QCoreApplication>
#include "tcpclient.h"
#include <qDebug>


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    TcpClient client;

    client.write();
    return a.exec();
}
