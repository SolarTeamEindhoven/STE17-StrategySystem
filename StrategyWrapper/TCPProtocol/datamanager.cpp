#include "datamanager.h"

DataManager::DataManager() {
    qDebug() << "Initializing datamanager...";
    server = new QTcpServer(this);
    connect(server, SIGNAL(newConnection()), SLOT(newConnection()));
    QList<QPair<Type, QString>> canSpec = serializer.getSpec(21);
    QList<QPair<Type, QString>> wfsSpec = serializer.getSpec(2, 2);
    QList<QPair<Type, QString>> stsSpec = serializer.getSpec(4, 4);
    QList<QPair<Type, QString>> ltsSpec = serializer.getSpec(5, 5);
    QList<QPair<Type, QString>> paramSpec = serializer.getSpec(6, 6);
    //dbHandler.setSpec(canSpec, wfsSpec, stsSpec, ltsSpec, paramSpec);

    setAndStartTimer();

    qDebug() << "Start listing to clients..." << server->listen(QHostAddress::SpecialAddress::Any, 5000);
    qDebug() << "Server IP" << server->serverAddress().toString();
    qDebug() << "Server port" << server->serverPort();
    qDebug() << "Datamanager initialized!";
}

void DataManager::setAndStartTimer() {
    timer.setInterval(DATAPERIOD); //TODO this may be changed
    connect(&timer, SIGNAL(timeout()), SLOT(timerCallBack()));

    quint64 lastTime = ((QDateTime::currentMSecsSinceEpoch()) / DATAPERIOD)*DATAPERIOD; //round them down to the period
    while (lastTime == ((QDateTime::currentMSecsSinceEpoch()) / DATAPERIOD)*DATAPERIOD); //round them down to the period
    //as soon as we pass period interval since epoch, we start the timer. Hereby we insure that the rounded off timestamps later on are as close as possible to the real timestamps.
    timer.start();

}

void DataManager::newConnection() {
    while (server->hasPendingConnections()) {
        QTcpSocket* socket = server->nextPendingConnection();
        SocketHandler handler(socket, this);
        connect(socket, SIGNAL(disconnected()), SLOT(disconnected()));
        socketHashMutex.lock();
        socketHash.insert(socket, handler);
        socketHash.find(socket).value().initializeConnects();
        socketHashMutex.unlock();
        qDebug() << "New client connected...";
    }
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

    socketHashMutex.lock();
    WriteHandler* writeHandler = &socketHash.find(hHandler->socket).value().writeHandler;
    socketHashMutex.unlock();
    //if clienttype is visualizer
    if(type == visualizer) {
        //if a client is a visualizer, it needs the last data fields, permanently
        connect(this, SIGNAL(newVisData(QByteArray&)), writeHandler, SLOT(sendVisData(QByteArray&)));
        connect(this, SIGNAL(newSTSData(quint32, bool)), writeHandler, SLOT(sendRows(quint32, bool)));
        connect(this, SIGNAL(newLTSData(quint32, bool)), writeHandler, SLOT(sendRows(quint32, bool)));
        connect(this, SIGNAL(newParamData(quint32, bool)), writeHandler, SLOT(sendRows(quint32, bool)));
        connect(this, SIGNAL(newWFSData(quint32, bool)), writeHandler, SLOT(sendRows(quint32, bool)));

        writeHandler->sendRows(1, false);
        writeHandler->sendRows(2, true);
        writeHandler->sendRows(4, true);
        writeHandler->sendRows(5, false);
        writeHandler->sendRows(6, false);
        qDebug() << "Client has identified itself as visualizer";
    }
    else if(type == weather) {
        writeHandler->sendRows(4, true);
        qDebug() << "Client has identified itself as WFS";
    }
    else if(type == strategy) {
        writeHandler->sendRows(2, true);
        writeHandler->sendRows(6, false);
        writeHandler->sendRows(7, false);
        qDebug() << "Client has identified itself as strategy model";
    }
    hHandler->clientType = type;
}

void DataManager::newField(quint32 id, quint32 dataSize, QTcpSocket* socket, bool multipleLines) {
    serializer.readAndChop(socket, id, dataSize, multipleLines); //write the datablock to the handler
    //if it's a non periodic message, emit them to all visualizers
    if (id < 10) {
        if (id == 2) { //emit the WFS data to all visualizers
            emit newWFSData(2, true);
            qDebug() << "New WFS data emitted";
        }
        else if (id == 4) { //emit the STS data to all visualizers
           emit newSTSData(4, true);
           qDebug() << "New STS data emitted";
        }
        else if (id == 5) { //emit the LTS data to all visualizers
            emit newLTSData(5, false);
            qDebug() << "New LTS data emitted";
        }
        else if (id == 6) { //emit the Param data to all visualizers
            emit newParamData(6, false);
            qDebug() << "New params data emitted";
        }
    }
}

void DataManager::sendField(quint32 id, QTcpSocket* socket, bool multipleLines) {
    serializer.sendField(id, socket, multipleLines);
}

void DataManager::timerCallBack() {
    if (serializer.checkNewData()) {
        union {
            char bytes[4];
            quint32 value;
        } id;
        union {
            char bytes[8];
            quint64 value;
        } timestamp;
        id.value = qToLittleEndian(1);
        timestamp.value = qToLittleEndian(((QDateTime::currentMSecsSinceEpoch() + DATAPERIOD/2) / DATAPERIOD)*DATAPERIOD); //rounded to the next interval

        QByteArray data(id.bytes);
        data.append(timestamp.bytes);
        data.append(serializer.sendVisData());
        emit newVisData(data);
    }
}
