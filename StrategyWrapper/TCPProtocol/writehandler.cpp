#include "datamanager.h" //we need to have sockethandlers functions

WriteHandler::WriteHandler(QTcpSocket* socket, DataManager* dataManager, SocketHandler* parent) :
    socket(socket), dataManager(dataManager), socketHandler(parent), bufferLines(0)
{}

void WriteHandler::initializeConnects() {
    flushTimer.setInterval(500); //TODO this may be changed
    connect(&flushTimer, SIGNAL(timeout()), SLOT(flush()),Qt::DirectConnection);
    connect(this, SIGNAL(sendVisDataSignal()), this, SLOT(sendVisDataSlot()),Qt::QueuedConnection);
    flushTimer.start();
}

/*
 * This function is executed in the timer thread. Hence the sendVisData signal, in order to switch to our own thread.
 */
void WriteHandler::addVisData(QByteArray& data) {
    bufferMutex.lock();
    visLinesBuffer.append(data); //add them to the data
    bufferLines++;
    bufferMutex.unlock();
    emit sendVisDataSignal();
    qDebug() << "Finished updating buffer" << bufferLines;
    bufferLines++;
}

void WriteHandler::sendVisDataSlot() {
    writeMutex.lock();
    bufferMutex.lock();
    if (!visLinesBuffer.isEmpty()) {
        socket->write(visLinesBuffer);
        visLinesBuffer.clear();
    }
    bufferMutex.unlock();
    writeMutex.unlock();
}

//This is executed in the sockets thread
void WriteHandler::sendRows(quint32 id) {
    writeMutex.lock();
    dataManager->sendField(id, socket);
    writeMutex.unlock();
}

void WriteHandler::flush() {
    writeMutex.lock();
    socket->flush();
    writeMutex.unlock();
}
