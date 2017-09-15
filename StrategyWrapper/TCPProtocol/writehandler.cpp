#include "datamanager.h" //we need to have sockethandlers functions

WriteHandler::WriteHandler(QTcpSocket* socket, DataManager* dataManager, SocketHandler* parent) :
    socket(socket), dataManager(dataManager), socketHandler(parent)
{}

void WriteHandler::initializeConnects() {
    flushTimer.setInterval(500); //TODO this may be changed
    connect(&flushTimer, SIGNAL(timeout()), SLOT(flush()));
    flushTimer.start();
}

void WriteHandler::sendVisData(QByteArray& data) {
    if (mutex.tryLock()) {
        if (!collectedVisRows.isEmpty()) {
            socket->write(collectedVisRows);
            collectedVisRows.clear();
        }
        socket->write(data);
        mutex.unlock();
    }
    else {
        collectedVisRows.append(data);
    }
}

void WriteHandler::sendRows(quint32 id, bool multipleLines) {
    mutex.lock();
    dataManager->sendField(id, socket, multipleLines);
    mutex.unlock();
}

void WriteHandler::flush() {
    mutex.lock();
    socket->flush();
    mutex.unlock();
}
