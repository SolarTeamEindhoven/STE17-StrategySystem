#ifndef TCPMESSAGE_H
#define TCPMESSAGE_H

#include <QtCore>
#include <QtEndian>

class TCPMessage
{
public:
    TCPMessage(quint8 typeID, quint16 streamID);
    quint8 typeID;
    quint16 streamID;
};

class TCPMessage1: public TCPMessage
{
public:
    TCPMessage1(quint16 streamID, bool subscribe);
    void decrypt(QByteArray& array);
    QByteArray encrypt ();

    bool subscribe;

};

class TCPMessage2: public TCPMessage
{
public:
    TCPMessage2(quint16 streamID, quint64 timeStamp1, quint64 timeStamp2);
    void decrypt(QByteArray& array);
    QByteArray encrypt ();

    quint64 timeStamp1; //ms since 1 jan 1970
    quint64 timeStamp2; //ms since 1 jan 1970

};

template<typename CanStruct>
class TCPMessage3: public TCPMessage {
public:
    TCPMessage3(quint16 streamID) :
        TCPMessage(3, streamID) {}

    void decryptHeader(QByteArray& header);
    void decryptData(QByteArray& data);
    QByteArray encrypt ();

    quint32 size;

    QList<QPair<quint64, CanStruct>> dataBlock;
};


#endif // TCPMESSAGE_H
