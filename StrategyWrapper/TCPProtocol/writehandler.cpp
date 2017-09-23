#include "datamanager.h" //we need to have sockethandlers functions

WriteHandler::WriteHandler(QTcpSocket* socket, DataManager* dataManager, SocketHandler* parent) :
    socket(socket), dataManager(dataManager), socketHandler(parent), bufferLines(0)
{}

void WriteHandler::initializeConnects() {
    flushTimer.moveToThread(this->thread());
    flushTimer.setInterval(500); //TODO this may be changed
    connect(&flushTimer, SIGNAL(timeout()), SLOT(flush()),Qt::DirectConnection);
    connect(this, SIGNAL(sendRowsInternalSignal(quint32)), this, SLOT(sendRows(quint32)), Qt::QueuedConnection);
    connect(this, SIGNAL(sendVisDataSignal()), this, SLOT(sendVisDataSlot()));
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
}

void WriteHandler::sendVisDataSlot() {
    writeMutex.lock();
    bufferMutex.lock();
    if (!visLinesBuffer.isEmpty()) {
        if (bufferLines > 1) {
            qDebug() << "BufferLines " << bufferLines;
        }
        socket->write(visLinesBuffer);
        socket->flush();
        visLinesBuffer.clear();
        bufferLines = 0;
    }
    bufferMutex.unlock();
    writeMutex.unlock();
}

//This is executed in the sockets thread
void WriteHandler::sendRows(quint32 id) {
    writeMutex.lock();
    //qDebug() << "Sending data " << QThread::currentThread() << this->thread() << socket->thread();
    dataManager->sendField(id, socket);
    //qDebug() << "Sent data" << QThread::currentThread();
    writeMutex.unlock();
}

//this function may be called by other threads
void WriteHandler::sendRowsRequest(quint32 id) {
    emit sendRowsInternalSignal(id);
}

void WriteHandler::flush() {
    writeMutex.lock();
    socket->flush();
    writeMutex.unlock();
}
