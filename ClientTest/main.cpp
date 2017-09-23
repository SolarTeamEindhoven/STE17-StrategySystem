#include <QCoreApplication>
#include "tcpclient.h"
#include <qDebug>


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    //QThread mk5thread;
    //mk5thread.start();
    //TcpClient mk5(1);
    //mk5.moveToThread(&mk5thread);

    //TcpClient wfs(2);

    TcpClient vis(3);
    vis.identify(1);

    /*TcpClient vis2(4);
    vis2.identify(1);

    TcpClient vis3(5);
    vis3.identify(1);*/

    //wfs.setTimer2();
    //mk5.setTimer();

    return a.exec();
}
