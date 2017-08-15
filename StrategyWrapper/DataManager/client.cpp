#include "client.h"


Client::Client(QObject *parent) : QObject(parent)
{
    socket = new QTcpSocket(this);
}

bool Client::connectToHost(QString host)
{
    //QHostAddress::LocalHost();
    socket->connectToHost(host, 1024);
    return socket->waitForConnected(30000);
}

bool Client::writeData(QByteArray data)
{
    if(socket->state() == QAbstractSocket::ConnectedState)
    {
        socket->write(intToArray(data.size())); //write size of data
        socket->write(data); //write the data itself
        return socket->waitForBytesWritten(1000);;
    }
    else
        return false;
}


QByteArray intToArray(qint32 source) //Use qint32 to ensure that the number have 4 bytes
{
    //Avoid use of cast, this is the Qt way to serialize objects
    QByteArray temp;
    QDataStream data(&temp, QIODevice::ReadWrite);
    data << source;
    return temp;
}
