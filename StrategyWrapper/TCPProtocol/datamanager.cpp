#include "datamanager.h"

DataManager::DataManager()
{
    qDebug() << "Initializing datamanager";
    server = new QTcpServer(this);

    connect(server, SIGNAL(newConnection()), SLOT(newConnection()));
    setAndStartTimer();
    serializer = Serializer::getInstance();
    qDebug() << "Datamanager initialized";

}

void DataManager::setAndStartTimer() {
    timer.setInterval(DATAPERIOD); //TODO this may be changed
    connect(&timer, SIGNAL(timeout()), SLOT(timerCallBack()));

    quint64 lastTime = ((QDateTime::currentMSecsSinceEpoch()) / DATAPERIOD)*DATAPERIOD; //round them down to the period
    quint64 thisTime = ((QDateTime::currentMSecsSinceEpoch()) / DATAPERIOD)*DATAPERIOD; //round them down to the period
    while (lastTime == thisTime) {
        thisTime == ((QDateTime::currentMSecsSinceEpoch()) / DATAPERIOD)*DATAPERIOD; //round them down to the period
    }
    //as soon as we pass period interval since epoch, we start the timer. Hereby we insure that the rounded off timestamps later on are as close as possible to the real timestamps.
    timer.start();

}

void DataManager::newConnection() {
    qDebug() << "New client tries to connect";
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    SocketHandler handler(socket, this);

    socketHashMutex.lock();
    socketHash.insert(socket, handler);
    connect(socket, SIGNAL(disconnected()), SLOT(disconnected()));
    socketHashMutex.unlock();
}

void DataManager::disconnected() {
    qDebug() << "Client disconnects";
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());

    socketHashMutex.lock();
    socketHash.remove(socket);
    socketHashMutex.unlock();
}

void DataManager::newClientType(ClientType type) {
    HeaderHandler* hHandler = qobject_cast<HeaderHandler*>(sender());

    WriteHandler* writeHandler = &hHandler->parent->writeHandler;
    //if clienttype is visualizer
    if(type == visualizer) {
        qDebug() << "Client has identified itself as visualizer";
        //if a client is a visualizer, it needs the last data fields, permanently
        connect(this, SIGNAL(newVisData(quint32, QByteArray&)), writeHandler, SLOT(sendRow(quint32, QByteArray&)));
        connect(this, SIGNAL(newSTSData(quint32, quint32, QByteArray&)), writeHandler, SLOT(sendRows(quint32,quint32,QByteArray&)));
        connect(this, SIGNAL(newLTSData(quint32, QByteArray&)), writeHandler, SLOT(sendRow(quint32,QByteArray&)));
        connect(this, SIGNAL(newParamData(quint32, QByteArray&)), writeHandler, SLOT(sendRow(quint32,QByteArray&)));
        connect(this, SIGNAL(newWFSData(quint32,quint32,QByteArray&)), writeHandler, SLOT(sendRows(quint32,quint32, QByteArray&)));

        //send all current last data
        QByteArray vis = serializer->getVisData();
        QPair<quint32, QByteArray> wfs = serializer->getField(2);
        QPair<quint32, QByteArray> sts = serializer->getField(4);
        QPair<quint32, QByteArray> lts = serializer->getField(5);
        QPair<quint32, QByteArray> params = serializer->getField(6);

        writeHandler->sendRow(1,vis);
        writeHandler->sendRows(2, wfs.first, wfs.second);
        writeHandler->sendRows(4, sts.first, sts.second);
        writeHandler->sendRow(5, lts.second);
        writeHandler->sendRow(6, params.second);
    }
    else if(type == weather) {
        qDebug() << "Client has identified itself as WFS";
        QPair<quint32, QByteArray> sts = serializer->getField(4);
        writeHandler->sendRows(4, sts.first, sts.second);
    }
    else if(type == strategy) {
        qDebug() << "Client has identified itself as strategy model";
        QPair<quint32, QByteArray> wfs = serializer->getField(2);
        QPair<quint32, QByteArray> params = serializer->getField(6);
        QByteArray strat = serializer->getStratCANData();


        writeHandler->sendRows(2, wfs.first, wfs.second);
        writeHandler->sendRow(6, params.second);
        writeHandler->sendRow(7, strat);
    }
    hHandler->clientType = type;
}

void DataManager::newField(quint32 id, quint32 dataSize, QTcpSocket* socket, bool multipleLines) {
    serializer->readAndChop(socket, id, dataSize, multipleLines); //write the datablock to the handler

    //if it are non periodic canmsgs, emit them to all visualizers
    if (id < 10) {
        if (id == 2) { //emit the WFS data to all visualizers
            qDebug() << "New WFS data emitted";
            QPair<quint32, QByteArray> wfs = serializer->getField(2);
            emit newWFSData(2, wfs.first, wfs.second);
        }
        else if (id == 4) { //emit the STS data to all visualizers
           qDebug() << "New STS data emitted";
           QPair<quint32, QByteArray> sts = serializer->getField(4);
           emit newSTSData(4, sts.first, sts.second);
        }
        else if (id == 5) { //emit the LTS data to all visualizers
            qDebug() << "New LTS data emitted";
            QPair<quint32, QByteArray> lts = serializer->getField(5);
            emit newLTSData(5, lts.second);
        }
        else if (id == 6) { //emit the Param data to all visualizers
            qDebug() << "New params data emitted";
            QPair<quint32, QByteArray> params = serializer->getField(6);
            emit newParamData(6, params.second);
        }
    }
}

void DataManager::timerCallBack() {
    if (serializer->checkNewData()) {
        QByteArray vis = serializer->getVisData();
        emit newVisData(1, vis);
    }
}
