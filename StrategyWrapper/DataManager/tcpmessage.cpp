#include "tcpmessage.h"

TCPMessage::TCPMessage(quint8 typeID, quint16 streamID)
{
    this->typeID = typeID;
    this->streamID = streamID;

}

//---------------------------------------------

TCPMessage1::TCPMessage1(quint16 streamID, bool subscribe) :
    TCPMessage(1, streamID)
{
    this->subscribe = subscribe;
}

//encrypts the msg1 protocol
QByteArray TCPMessage1::encrypt() {
    QByteArray temp;
    QDataStream data(&temp, QIODevice::ReadWrite);
    data << qToLittleEndian(typeID);
    data << qToLittleEndian(streamID);
    data << qToLittleEndian(subscribe);
    return temp;
}

void TCPMessage1::decrypt(QByteArray& array) {
    QDataStream data(&array, QIODevice::ReadWrite);
    data >> qFromLittleEndian(typeID);
    data >> qFromLittleEndian(streamID);
    data >> qFromLittleEndian(subscribe);
}

//----------------------------------------------

TCPMessage2::TCPMessage2(quint16 streamID, quint64 timeStamp1, quint64 timeStamp2) :
    TCPMessage(2, streamID)
{
    this->timeStamp1 = timeStamp1;
    this->timeStamp2 = timeStamp2;
}

//encrypts the msg1 protocol
QByteArray TCPMessage2::encrypt() {
    QByteArray temp;
    QDataStream data(&temp, QIODevice::ReadWrite);
    data << qToLittleEndian(typeID);
    data << qToLittleEndian(streamID);
    data << qToLittleEndian(timeStamp1);
    data << qToLittleEndian(timeStamp2);
    return temp;
}

void TCPMessage2::decrypt(QByteArray& array) {
    QDataStream data(&array, QIODevice::ReadWrite);
    data >> fromLittleEndian(typeID;
    data >> fromLittleEndian(streamID);
    data >> fromLittleEndian(timeStamp1;
    data >> timeStamp2;
}

//----------------------------------------------

TCPMessage3::TCPMessage2(quint16 streamID) :
    TCPMessage(3, streamID)
{}

//encrypts the msg1 protocol
QByteArray TCPMessage3::encrypt() {
    QByteArray temp;
    QDataStream data(&temp, QIODevice::ReadWrite);
    data << streamID;
    data << typeID;
    return temp;
}
