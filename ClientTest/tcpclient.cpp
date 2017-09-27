#include "tcpclient.h"

TcpClient::TcpClient(quint32 number) : number(number), id(21), size(0), type_id(0), readingType(true)
{
    qDebug() << "client "<< number << "Connecting...";
    connect(&socket, SIGNAL(readyRead()), SLOT(newMessage()));
    QHostAddress address("192.168.1.153");
    socket.setParent(this);
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
    while(1) {
        if (readingType) {
            if (socket.bytesAvailable() >= 8) {
                union{
                    char bytes[sizeof(quint32)];
                    quint32 value;
                }id1, size2;
                socket.read(id1.bytes,4);
                id1.value = qFromLittleEndian(id1.value);
                if (id1.value == 1) {
                    union{
                        char bytes[sizeof(quint64)];
                        quint64 value;
                    } timestamp;
                    socket.read(timestamp.bytes,8);
                    timestamp.value = qFromLittleEndian(timestamp.value);
                    qDebug() << "New vismessage found" << timestamp.value;
                    size = 220;
                    readingType = false;
                }
                else if (id1.value == 2 || id1.value == 4) {
                    if (id1.value == 2) {
                        qDebug() << "New WFS message found";
                    }
                    else {
                        qDebug() << "New STS message found";
                    }
                    socket.read(size2.bytes,4);
                    size = qFromLittleEndian(size2.value);
                    readingType = false;
                }
                else if (id1.value == 5) {
                    qDebug() << "New LTS found";
                    size = 15;
                    readingType = false;
                }
                else if (id1.value == 6) {
                    qDebug() << "New params found";
                    size = 136;
                    readingType = false;
                }
                else {
                    qDebug() << id1.value << "Hellup";
                }

            }
            else {
                return;
            }
        }
        else {
            if (socket.bytesAvailable() >= size) {
                QByteArray array = socket.read(size);
                readingType = true;
                qDebug() << "read " << size << "bytes";
            }
            else {
                return;
            }
        }
    }
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

    int numberOfRows = (8*36*29*1024)/8;
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
        array.append(float1.bytes, sizeof(float));
        array.append(float2.bytes, sizeof(float));
    }
    qDebug() << array.length();
    socket.write(id.bytes,4);
    socket.write(size.bytes,4);
    socket.write(array);
    socket.flush();
    qDebug() << "client "<< number << "Sent big message MAKE THIS STRING LONGER SO IT STANDS OUT OF OTHER MESSAGES";
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

    for (quint32 j = 0; j < 130; j++) {
        if (this->id >= 2030) {
            this->id = 21;
        }
        using T = float;
        union {
            char bytes[sizeof(T)];
            T value;
        } pi;
        id1.value = qToLittleEndian(this->id);
        pi.value = qToLittleEndian<T>(3.1415);
        socket.write(id1.bytes,4);
        for (int i = 0; i < 16/sizeof(T); i++) {
            socket.write(pi.bytes,sizeof(T));
        }
        //socket.flush();
        this->id++;
    }
    //qDebug() << "Sent 15 messages";

}

void TcpClient::setTimer2() {
    timer2.setInterval(20000); //TODO this may be changed
    connect(&timer2, SIGNAL(timeout()), SLOT(writeLargeData()));
    timer2.start();
}

void TcpClient::setTimer() {
    timer.setInterval(15); //TODO this may be changed
    connect(&timer, SIGNAL(timeout()), SLOT(timerCheckout()));
    timer.start();
}
