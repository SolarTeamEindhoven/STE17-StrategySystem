#include "tcpclient.h"

TcpClient::TcpClient()
{
    qDebug() << "Connecting...";
    connect(&socket, SIGNAL(readyRead()), SLOT(newMessage()));
    socket.connectToHost(QHostAddress::SpecialAddress::LocalHost,5000);
    if(!socket.waitForConnected()) {
        qDebug() << socket.error();
    }
}

void TcpClient::write() {
    union{
        char bytes[sizeof(quint32)];
        quint32 value;
    }id, useless;
    union {
        char bytes[sizeof(float)];
        float value;
    }float1, float2;
    quint32 idnumber = 11;
    id.value = qToLittleEndian(idnumber);
    socket.write(id.bytes,4);
    float1.value = 0.23;
    float2.value = 4.3;
    socket.write(float1.bytes,4);
    socket.write(float2.bytes,4);
    useless.value = 1;
    socket.write(useless.bytes,4);
    socket.write(useless.bytes,4);
    socket.flush();
}

void TcpClient::newMessage() {
    qDebug() << "New Message arrived";
}
