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
        char bytes[sizeof(quint8)];
        quint8 value;
    }type;

    union{
        char bytes[1];
        bool value;
    }subscribe;

    union{
        char bytes[sizeof(quint32)];
        quint32 value;
    }id1, id, useless, size;

    union {
        char bytes[sizeof(float)];
        float value;
    }float1, float2;

    quint32 msgType = 0;
    id1.value = qToLittleEndian(msgType);
    quint8 clientType = 1;
    type.value = qToLittleEndian(clientType);
    bool sub = true;
    subscribe.value = qToLittleEndian(sub);

    socket.write(id1.bytes,4);
    socket.write(type.bytes,1);
    socket.write(subscribe.bytes,1);
    socket.flush();

    QTime dieTime = QTime::currentTime().addMSecs( 1000 );
    while( QTime::currentTime() < dieTime )
    {
        QCoreApplication::processEvents( QEventLoop::AllEvents, 100 );
    }

    quint32 sizenumber = 80;
    quint32 idnumber = 2;
    size.value = qToLittleEndian(sizenumber);
    id.value = qToLittleEndian(idnumber);
    socket.write(id.bytes,4);
    socket.write(size.bytes,4);
    for (int i = 0; i < 10; i++) {
        float1.value = 0.23 + i;
        float2.value = 4.3 + i;
        socket.write(float1.bytes,4);
        socket.write(float2.bytes,4);
    }
    socket.flush();
    qDebug() << "Sent visualizer authentication";
}

void TcpClient::newMessage() {
    qDebug() << "New Message arrived" << socket.bytesAvailable() << "bytes";
    if (socket.bytesAvailable() >= 16) {
        union {
            char bytes[8];
            quint64 value;
        } timestamp;
        union{
            char bytes[sizeof(quint32)];
            quint32 value;
        }id;
        union {
            char bytes[sizeof(float)];
            float value;
        }float1;
        socket.read(id.bytes,4);
        socket.read(timestamp.bytes,8);
        socket.read(float1.bytes,4);
    }
    qDebug() << "Visualizer message accepted!";
}
