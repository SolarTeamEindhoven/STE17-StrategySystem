#include "sockethandler.h" //we need to have sockethandlers functions

WriteHandler::WriteHandler(QTcpSocket* socket, DataManager* dataManager, SocketHandler* parent) :
    socket(socket), dataManager(dataManager), parent(parent)
{}

void WriteHandler::initializeConnects() {
    flushTimer.setInterval(200); //TODO this may be changed
    connect(&flushTimer, SIGNAL(timeout()), SLOT(flush()));
    flushTimer.start();
}

void WriteHandler::sendRows(quint32 id, quint32 size, QByteArray& data) {
    union {
        quint32 value;
        char bytes[sizeof(quint32)];
    }idUnion, sizeUnion;

    idUnion.value = qToLittleEndian(id);
    sizeUnion.value = qToLittleEndian(size);

    parent->mutex.lock();
    socket->write(idUnion.bytes, sizeof(quint32));
    socket->write(sizeUnion.bytes, sizeof(quint32));
    socket->write(data);
    parent->mutex.unlock();
}
void WriteHandler::sendRow(quint32 id, QByteArray& data) {
    union {
        quint32 value;
        char bytes[sizeof(quint32)];
    }idUnion;

    idUnion.value = qToLittleEndian(id);
    parent->mutex.lock();
    socket->write(idUnion.bytes, sizeof(quint32));
    socket->write(data);
    parent->mutex.unlock();
}

void WriteHandler::flush() {
    parent->mutex.lock();
    socket->flush();
    parent->mutex.unlock();
}
