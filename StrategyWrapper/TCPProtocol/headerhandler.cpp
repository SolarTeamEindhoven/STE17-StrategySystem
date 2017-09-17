#include "datamanager.h" //a bit of a way around, but I need the DM definition here

HeaderHandler::HeaderHandler(QTcpSocket* socket, DataManager* dataManager, SocketHandler* parent) : QObject(nullptr),
    clientType(unclear), socket(socket), dataManager(dataManager), socketHandler(parent), state(readType), id(0), size(0)
{}

HeaderHandler::HeaderHandler(const HeaderHandler& copy) : QObject(copy.parent()),
    clientType(copy.clientType), socket(copy.socket), dataManager(copy.dataManager), socketHandler(copy.socketHandler),
    state(copy.state), id(copy.id), size(copy.size)
{}

HeaderHandler::~HeaderHandler() {
}

void HeaderHandler::readyRead() {
    switch(state) {
    case HHState::readType :
        readNextType();
        break;
    case HHState::readData :
        readNextData();
        break;
    case HHState::readSize :
        readNextSize();
        break;
    case HHState::readNewClient :
        readNextNewClient();
        break;
    default:
        break;
    }
}

void HeaderHandler::readNextType() {
    if (socket->bytesAvailable() >= 4) { //check if there are enough bytes available

        union {
            char bytes[sizeof(quint32)];
            quint32 value;
        } data;
        socket->read(data.bytes, sizeof(quint32));
        id = qFromLittleEndian(data.value);
        //interpret the id to determine what's next
        if (id >= 10 && id < 2048) {
            char bytes[8];
            socket->read(bytes, 8); //throw away the timestamp, we're gonna add a new timestamp
            size = 8; //8 bytes candata
            state = readData;
            readNextData();
        }
        else if (id == 5) {
            size = 16;
            state = readData;
            readNextData();
        }
        else if (id == 6) {
            size = 18; //TODO: change this
            state = readData;
            readNextData();
        }
        else if (id == 7) {
            size = 20;
            state = readData;
            readNextData();
        }
        else if (id == 0) {
            state = readNewClient;
            readNextNewClient();
        }
        else if (id == 2 || id == 4) { //id's that I expect are: 2, 4
            state = readSize;
            readNextSize();
        }
        else {
            //This ID shouldn't be sent, something went wrong
            qDebug() << "Error, protocol breach at" << socket->peerAddress() << ", " << socket->peerPort();
            qDebug() << "Read ID of " <<id;
            qDebug() << "Closing socket...";
            socket->disconnectFromHost();
            socket->waitForDisconnected();
            emit closeSocket(socket);
        }
    }
    //else wait for next readyread signal
}

void HeaderHandler::readNextSize() {
    if (socket->bytesAvailable() >= 4) { //check if there are enough bytes available
        union {
            char bytes[sizeof(quint32)];
            quint32 value;
        } data;
        socket->read(data.bytes, sizeof(quint32));
        size = qFromLittleEndian(data.value);
        state = readData;
        readNextData();
    }
    //else wait for next readyread signal
}

void HeaderHandler::readNextData() {
    if (socket->bytesAvailable() >= size) { //check if there are enough bytes available
        dataManager->newField(id, size, socket);
        state = readType;
        readNextType();
    }
    //else wait for next readyread signal
}

void HeaderHandler::readNextNewClient() {
    if (socket->bytesAvailable() >= 2) {
        union {
            char bytes[sizeof(quint8)];
            quint8 value;
        } type;

        union {
            char bytes[sizeof(bool)];
            bool value;
        } subscribe;

        socket->read(type.bytes, sizeof(quint8));
        socket->read(subscribe.bytes, sizeof(bool));
        type.value = qFromLittleEndian(type.value);
        if (subscribe.value) {
            switch(type.value) {
            case 1:
                emit newClientType(visualizer, socket);
                break;
            case 2:
                emit newClientType(strategy, socket);
                break;
            case 3:
                emit newClientType(weather, socket);
                break;
            default:
                //This ID shouldn't be sent, something went wrong
                qDebug() << "Error, protocol breach at" << socket->peerAddress() << ", " << socket->peerPort();
                qDebug() << "Read new subscribtion with type " << type.value;
                qDebug() << "Closing socket...";
                socket->disconnectFromHost();
                socket->waitForDisconnected();
                emit closeSocket(socket);
                break;
            }
        }
        state = readType;
        readNextType();
    } //else wait for next readyread signal
}

void HeaderHandler::initializeConnects() {
    socket->setParent(this); //move it with us to another thread
    connect(this, SIGNAL(newClientType(ClientType, QTcpSocket*)), dataManager, SLOT(newClientType(ClientType, QTcpSocket*)), Qt::DirectConnection);
    connect(this->socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
    connect(this, SIGNAL(closeSocket(QTcpSocket*)), dataManager, SLOT(removeSocket(QTcpSocket*)), Qt::QueuedConnection);
}
