#include "tcpclient.h"

TcpClient::TcpClient(quint32 number) : number(number), id(21)
{
    qDebug() << "client "<< number << "Connecting...";
    connect(&socket, SIGNAL(readyRead()), SLOT(newMessage()));
    QHostAddress address("192.168.1.128");
    socket.connectToHost(address,5000);
    if(!socket.waitForConnected()) {
        qDebug() << socket.error();
    }
}

void TcpClient::identify(quint32 type_id) {

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


    quint32 msgType = 0;
    id1.value = qToLittleEndian(msgType);
    quint8 clientType = type_id;
    type.value = qToLittleEndian(clientType);
    bool sub = true;
    subscribe.value = qToLittleEndian(sub);

    socket.write(id1.bytes,4);
    socket.write(type.bytes,1);
    socket.write(subscribe.bytes,1);
    socket.flush();

    qDebug() << "client "<< number << "Sent vis authentication";
}

void TcpClient::newMessage() {
    qDebug() << "client "<< number <<  "New Message arrived" << socket.bytesAvailable() << "bytes";
}

void TcpClient::writeLargeData() {
    union {
        char bytes[sizeof(float)];
        float value;
    }float1, float2;

    union{
        char bytes[sizeof(quint32)];
        quint32 value;
    }id1, id, useless, size;

    int numberOfRows = 8*36*29*1024/8;
    quint32 sizenumber = numberOfRows*8;
    quint32 idnumber = 2;
    size.value = qToLittleEndian(sizenumber);
    id.value = qToLittleEndian(idnumber);
    QByteArray array;
    for (int i = 0; i < numberOfRows; i++) {
        float1.value = 0.23 + i;
        float2.value = 4.3 + i;
        float1.value = qToLittleEndian(float1.value);
        float2.value = qToLittleEndian(float2.value);
        array.append(float1.bytes);
        array.append(float2.bytes);
    }
    qDebug() << "client "<< number << "Sending big message";
    socket.write(id.bytes,4);
    socket.write(size.bytes,4);
    socket.write(array);
    socket.flush();
    qDebug() << "client "<< number << "Sent big message";
    QTime dieTime = QTime::currentTime().addMSecs( 5000 );
    while( QTime::currentTime() < dieTime )
    {
        QCoreApplication::processEvents( QEventLoop::AllEvents, 100 );
    }
}

void TcpClient::timerCheckout() {
    union{
        char bytes[sizeof(quint32)];
        quint32 value;
    }id1, id, useless, size;

    for (int j = 0; j < 15; j++) {
        id1.value = qToLittleEndian(this->id);
        useless.value = 0;
        socket.write(id1.bytes,4);
        socket.write(useless.bytes,4);
        socket.write(useless.bytes,4);
        socket.write(useless.bytes,4);
        socket.write(useless.bytes,4);
        this->id++;
    }
    //socket.flush();
    if (this->id >= 2030) {
        this->id = 21;
    }
    qDebug() << "Sent 15 messages";

}

void TcpClient::setTimer() {
    timer.setInterval(1); //TODO this may be changed
    connect(&timer, SIGNAL(timeout()), SLOT(timerCheckout()));
    timer.start();
}
