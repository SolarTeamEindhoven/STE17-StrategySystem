#include "datamanager.h" //a bit of a way around, but I need the DM definition here

HeaderHandler::HeaderHandler(QTcpSocket* socket, DataManager* dataManager, SocketHandler* parent) :
    clientType(unclear), socket(socket), dataManager(dataManager), socketHandler(parent), state(readType), id(0), size(0), multipleLines(false)
{}

HeaderHandler::HeaderHandler(const HeaderHandler& copy) : QObject(copy.parent()),
    clientType(copy.clientType), socket(copy.socket), dataManager(copy.dataManager), socketHandler(copy.socketHandler),
    state(copy.state), id(copy.id), size(copy.size), multipleLines(copy.multipleLines)
{}

void HeaderHandler::initializeConnects() {
    connect(this, SIGNAL(newClientType(ClientType)), dataManager, SLOT(newClientType(ClientType)));
    connect(this->socket, SIGNAL(readyRead()), SLOT(readyRead()));
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
        if (id >= 10) {
            size = 16; //read 8 bytes timestamp, 8 bytes candata
            multipleLines = false;
            state = readData;
            readNextData();
        }
        else if (id == 5) {
            size = 16;
            multipleLines = false;
            state = readData;
            readNextData();
        }
        else if (id == 6) {
            size = 18; //TODO: change this
            multipleLines = false;
            state = readData;
            readNextData();
        }
        else if (id == 7) {
            size = 20;
            multipleLines = false;
            state = readData;
            readNextData();
        }
        else if (id == 0) {
            state = readNewClient;
            readNextNewClient();
        }
        else { //id's that I expect are: 2, 4
            state = readSize;
            readNextSize();
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
        multipleLines = true; //in general, these messages consist of multiple lines
        state = readData;
        readNextData();
    }
    //else wait for next readyread signal
}

void HeaderHandler::readNextData() {
    if (socket->bytesAvailable() >= size) { //check if there are enough bytes available
        dataManager->newField(id, size, socket, multipleLines);
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
                emit newClientType(visualizer);
                break;
            case 2:
                emit newClientType(strategy);
                break;
            case 3:
                emit newClientType(weather);
                break;
            default:
                break;
            }
        }
        state = readType;
        readNextType();
    } //else wait for next readyread signal
}
