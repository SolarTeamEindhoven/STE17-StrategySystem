#include "datamanager.h"

DataManager::DataManager() : visTimerThread(&visTimer), mk5(this) {
    qDebug() << "Initializing datamanager...";
    server = new QTcpServer(this);
    connect(server, SIGNAL(newConnection()), SLOT(newConnection()));
    QList<QPair<Type, QString>> canSpec = serializer.getSpec(21);
    QList<QPair<Type, QString>> wfsSpec = serializer.getSpec(2, 2);
    QList<QPair<Type, QString>> stsSpec = serializer.getSpec(4, 4);
    QList<QPair<Type, QString>> ltsSpec = serializer.getSpec(5, 5);
    QList<QPair<Type, QString>> paramSpec = serializer.getSpec(6, 6);

    qDebug() << "Main " << QThread::currentThread();
    //dbHandler.setSpec(canSpec, wfsSpec, stsSpec, ltsSpec, paramSpec);
    setAndStartTimer();

    qDebug() << "Start listing to clients..." << server->listen(QHostAddress::SpecialAddress::Any, 5000);
    qDebug() << "Server IP" << server->serverAddress().toString();
    qDebug() << "Server port" << server->serverPort();
    qDebug() << "Datamanager initialized!";
}

DataManager::~DataManager() {
}

void DataManager::setAndStartTimer() {
    visTimer.setInterval(DATAPERIOD); //TODO this may be changed
    visTimer.moveToThread(&visTimerThread);
    connect(&visTimer, SIGNAL(timeout()), SLOT(timerCallBack()), Qt::DirectConnection);
    quint64 lastTime = ((QDateTime::currentMSecsSinceEpoch()) / DATAPERIOD)*DATAPERIOD; //round them down to the period
    while (lastTime == ((QDateTime::currentMSecsSinceEpoch()) / DATAPERIOD)*DATAPERIOD); //round them down to the period
    //as soon as we pass period interval since epoch, we start the timer. Hereby we insure that the rounded off timestamps later on are as close as possible to the real timestamps.
    visTimerThread.start();
}

void DataManager::newConnection() {
    while (server->hasPendingConnections()) {
        QTcpSocket* socket = server->nextPendingConnection();
        SocketHandler handler(socket, this);
        connect(socket, SIGNAL(disconnected()), SLOT(disconnected()));
        socketHashMutex.lock();
        socketHash.insert(socket, handler);
        socketHash.find(socket).value().initializeConnects();
        //first assume it's an mk5
        socketHashMutex.unlock();
        qDebug() << "New client connected... IP:"
                 << socket->peerAddress().toString() << ", port :" << socket->peerPort();
    }
}

void DataManager::disconnected() {
    qDebug() << "Client disconnects";
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    removeSocket(socket);
}

void DataManager::removeSocket(QTcpSocket* socket) {
    socketHashMutex.lock();
    //disconnect everything
    socketHash.find(socket).value().writeHandler.socket->disconnect();
    socketHash.find(socket).value().readHandler.disconnect();
    socketHash.find(socket).value().writeHandler.disconnect();
    socketHash.find(socket).value().writeHandler.disconnect();
    disconnect(this, 0, &socketHash.find(socket).value().writeHandler, 0);
    //wait 5 seconds for the object to finish all its current tasks, else you're gonna get weird exceptions
    /*QTime dieTime = QTime::currentTime().addMSecs( 5000 );
    while( QTime::currentTime() < dieTime ) {
        QCoreApplication::processEvents( QEventLoop::AllEvents, 100 );
    }*/
    socketHashMutex.unlock();
}


void DataManager::newClientType(ClientType type, QTcpSocket* socket) {
    qDebug() << "Emitting new client type" << QThread::currentThreadId();
    socketHashMutex.lock();
    WriteHandler* writeHandler = &socketHash.find(socket).value().writeHandler;
    socketHashMutex.unlock();
    //if clienttype is visualizer
    if(type == visualizer) {
        //if a client is a visualizer, it needs the last data fields, permanently
        connect(this, SIGNAL(newVisData(QByteArray&)), writeHandler, SLOT(addVisData(QByteArray&)), Qt::DirectConnection);
        connect(this, SIGNAL(newSTSData(quint32)), writeHandler, SLOT(sendRowsRequest(quint32)));
        connect(this, SIGNAL(newLTSData(quint32)), writeHandler, SLOT(sendRowsRequest(quint32)));
        connect(this, SIGNAL(newParamData(quint32)), writeHandler, SLOT(sendRowsRequest(quint32)));
        //connect(this, SIGNAL(newWFSData(quint32)), writeHandler, SLOT(sendRowsRequest(quint32)));

        QByteArray visData = serializer.sendVisData();
        //writeHandler->addVisData(visData);
        //writeHandler->sendRowsRequest(2);
        writeHandler->sendRowsRequest(4);
        writeHandler->sendRowsRequest(5);
        writeHandler->sendRowsRequest(6);
        qDebug() << "Client has identified itself as visualizer. IP:"
                 << socket->peerAddress().toString()  << ", port :" << socket->peerPort();
    }
    else if(type == weather) {
        writeHandler->sendRowsRequest(4);
        qDebug() << "Client has identified itself as WFS";
    }
    else if(type == strategy) {
        writeHandler->sendRowsRequest(2);
        writeHandler->sendRowsRequest(6);
        writeHandler->sendRowsRequest(7);
        qDebug() << "Client has identified itself as strategy model";
    }
    socketHash.find(socket).value().readHandler.clientType = type;
}

void DataManager::newField(quint32 id, quint32 dataSize, QTcpSocket* socket) {
    serializer.readAndChop(socket, id, dataSize); //write the datablock to the handler
    //if it's a non periodic message, emit them to all visualizers
    if (id < 10) {
        if (id == 2) { //emit the WFS data to all visualizers
            emit newWFSData(2);
            qDebug() << "New WFS data emitted";
        }
        else if (id == 4) { //emit the STS data to all visualizers
           emit newSTSData(4);
           qDebug() << "New STS data emitted";
        }
        else if (id == 5) { //emit the LTS data to all visualizers
            emit newLTSData(5);
            qDebug() << "New LTS data emitted";
        }
        else if (id == 6) { //emit the Param data to all visualizers
            emit newParamData(6);
            qDebug() << "New params data emitted";
        }
    }
}

//this is executed in the sockets thread
void DataManager::sendField(quint32 id, QTcpSocket* socket) {
    serializer.sendField(id, socket);
}

void DataManager::timerCallBack() {
    //qDebug() << "Timer call";
    if (serializer.checkNewData()) {
        //qDebug() << "Sending vis message";
        QByteArray data = serializer.sendVisData();
        emit newVisData(data);
    }
    else {
        qDebug() << "No MK5 connection";
    }
}

