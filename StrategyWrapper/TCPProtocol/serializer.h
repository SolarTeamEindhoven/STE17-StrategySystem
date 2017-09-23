#ifndef CANSPEC_H
#define CANSPEC_H

#include <QtCore>
#include <QtEndian>
#include <QTcpSocket>
#include <QStringList>
#include <QFile>
#include <QDebug>
#include "tcpprotocol_global.h"
//TODO autogenerate this

#define TOTALIDs 2
#define DATAPERIOD 100 //in ms

enum Type {
    Float = 0,
    Other = 1,

    Int8 = 2,
    Int16 = 3,
    Int32 = 4,
    Int64 = 5,

    UInt8 = 6,
    UInt16 = 7,
    UInt32 = 8,
    UInt64 = 9,
    BitfieldBool = 10
};

struct LastDataStruct {
    LastDataStruct(Type type, bool toVis, bool toStrat, QString name) : type(type), toVis(toVis), toStrat(toStrat), dataSize(0), name(name) {}

    //make sure we don't copy the mutex
    LastDataStruct(const LastDataStruct& copy) :
        type(copy.type),
        dataField(copy.dataField),
        toVis(copy.toVis),
        toStrat(copy.toStrat),
        dataSize(copy.dataSize),
        name(copy.name) {}

    LastDataStruct& operator= (const LastDataStruct& data) {
        this->type = data.type;
        this->dataField = data.dataField;
        this->toVis = data.toVis;
        this->toStrat = data.toStrat;
        this->dataSize = data.dataSize;
        this->name = data.name;
        return *this;
    }
    Type type;
    QByteArray dataField;
    bool toVis;
    bool toStrat;
    quint32 dataSize;
    QString name;
};

struct FieldSerializer {
public:
    virtual void fromMk5Endianess(QByteArray* data, QTcpSocket* socket) = 0;
    virtual void toMk5Endianess(QByteArray* data, QTcpSocket* socket) = 0;
    virtual const quint16 getSize() = 0;
};

struct NoChange : public FieldSerializer {
public:
    NoChange(quint16 size) : size(size) {}

    void fromMk5Endianess(QByteArray* data, QTcpSocket* socket) override {
        *data = socket->read(size);
    }
    void toMk5Endianess(QByteArray* data, QTcpSocket* socket) override {}
    const quint16 getSize() override {
        return size;
    }
private:
    const quint16 size;
};

template <typename T>
struct SerializeInteger : public FieldSerializer {
public:
    void fromMk5Endianess(QByteArray* data, QTcpSocket* socket) override {
        QDataStream stream(socket);
        stream.setByteOrder(QDataStream::BigEndian);
        union {
            T value;
            char bytes[sizeof(T)];
        } type;
        stream >> type.value;
        *data = QByteArray(type.bytes, sizeof(T));
    }

    void toMk5Endianess(QByteArray* data, QTcpSocket* socket) override {
        QDataStream stream(data, QIODevice::ReadWrite);
    }

    const quint16 getSize() override {
        return sizeof(T);
    }
};

class TCPPROTOCOLSHARED_EXPORT Serializer {
public:
    Serializer() : noChange1(1), noChange4(4),
                   serialize{&noChange4, &noChange1, &noChange1, &uInt16, &uInt32, &uInt64, &noChange1, &int16, &int32, &int64},
                   lookUp{ 5000 }, newDataSinceLastCall(false), visMsgLength(0), stratMsgLength(0)
    {
        for (int i = 0; i < 2048; i++) {
            lookUp[i] = 5000;
        }
        initializeIds(); //this is autogenerated
        showSpec();
    }

    QList<QPair<quint32, QList<LastDataStruct>>>* getData() {
        return &dataStruct;
    }

    void readAndChop(QTcpSocket* socket, quint32 id, quint32 dataSize) {
        if (lookUp[id] != 5000) { //skip if id doesn't point to a spot in the qlist
            if (id < 10) {
                return storeDataBlocks(socket, id, dataSize);
            }
            QList<LastDataStruct>& fieldList = dataStruct[lookUp[id]].second;
            //read the first line
            canMutex.lock();
            for (int i = 0; i < fieldList.size() && dataSize > 0; i++) {
                fieldList[i].dataField.clear(); //clear the line
                quint16 thisSize = serialize[fieldList[i].type]->getSize();
                //fieldList[i].dataField = socket->read(thisSize); //append datafield to list
                serialize[fieldList[i].type]->fromMk5Endianess(&fieldList[i].dataField, socket);

                if (thisSize != fieldList[i].dataField.length()) {
                    qDebug() << "Something went wrong reading "<< id << i << thisSize << fieldList[i].dataField.length();
                }
                dataSize -= thisSize;
            }
            if (dataSize > 0) { //canmessages are sometimes shorter than 8 bytes, so throw away the useless data
                socket->read(dataSize);
            }
            /*if (!newDataSinceLastCall) {
                qDebug() << "New can message" << id;
            }*/
            newDataSinceLastCall = true;
            canMutex.unlock();
        }
        else { //if we don't know this one, assume it's some kind of canmessage and throw away 8 bytes
            char uselessData[8];
            socket->read(uselessData, 8);
        }
    }

    void storeDataBlocks(QTcpSocket* socket, quint32 id, quint32 dataSize) {
        QList<LastDataStruct>& fieldList = dataStruct[lookUp[id]].second;
        dataBlocksMutex.lock();
        fieldList[0].dataField.clear();
        fieldList[0].dataSize = dataSize;
        fieldList[0].dataField = socket->read(dataSize);
        dataBlocksMutex.unlock();
    }

    void sendStratCANData(QTcpSocket* socket) {
       canMutex.lock();
         for (int i = 0; i < dataStruct.size(); i++) {
            if (dataStruct[i].first >= 10) {
                for (int j = 0; j < dataStruct[i].second.size(); j++) {
                    LastDataStruct* str = &dataStruct[i].second[j];
                    if (str->toStrat && str->dataField.length() > 0) {
                        socket->write(str->dataField);
                    }
                }
            }
        }
        canMutex.unlock();
    }

    QByteArray sendVisData() {
        union {
            char bytes[4];
            quint32 value;
        } id;
        union {
            char bytes[8];
            quint64 value;
        } timestamp;
        id.value = qToLittleEndian<quint32>(1);
        timestamp.value = qToLittleEndian<quint64>(((QDateTime::currentMSecsSinceEpoch() + DATAPERIOD/2) / DATAPERIOD)*DATAPERIOD); //rounded to the next interval
        QByteArray data(id.bytes,4);
        data.append(timestamp.bytes,8);
        data.append(id.bytes,4); //distance... TODO: fix this
        canMutex.lock();
         for (int i = 0; i < dataStruct.size(); i++) {
            if (dataStruct[i].first >= 10) {
                for (int j = 0; j < dataStruct[i].second.size(); j++) {
                    LastDataStruct* str = &dataStruct[i].second[j];
                    if (str->toVis && str->dataField.length() > 0) {
                        data.append(str->dataField);
                    }
                }
            }
        }
        canMutex.unlock();
        if (data.length() != visMsgLength+4) {
            qDebug() << "Error vis length mismatch " <<data.length() << " " << visMsgLength+4; //+4 for ID
        }
        return data;
    }

    //this has been created to sendfields without having to copy data. It should therefore be faster
    //this is executed in the sockets thread. This will only be used for datablocks. Can will be sent via visualizer/strat messages.
    void sendField(quint32 id, QTcpSocket* socket) {
        //send the header
        union {
            quint32 value;
            char bytes[4];
        }idUnion, size;

        idUnion.value = id;
        idUnion.value = qToLittleEndian(idUnion.value);
        socket->write(idUnion.bytes,4); //write the id

        //if the data needs to be gathered in a non-trivial way, stop here and go to specialized functions
        if (id == 7) {
            return sendStratCANData(socket);
        }

        //send the datablock
        QList<LastDataStruct>& str = dataStruct[lookUp[id]].second;
        dataBlocksMutex.lock();
        //if size needs to be sent
        if (id == 2 || id == 4) {
            size.value = str[0].dataSize;
            size.value = qToLittleEndian(size.value);
            socket->write(size.bytes,4); //write the size
        }
        socket->write(str[0].dataField,str[0].dataSize);
        dataBlocksMutex.unlock();
    }

    bool checkNewData() {
        if (newDataSinceLastCall) {
            newDataSinceLastCall = false;
            return true;
        }
        return false;
    }

    QList<QPair<Type, QString>> getSpec(int beginId) {
        return getSpec(beginId, dataStruct.last().first); //return the spec until the last field
    }

    QList<QPair<Type, QString>> getSpec(int beginId, int endId) {
        QList<QPair<Type, QString>> spec;
        for (int i = lookUp[beginId]; i <= lookUp[endId]; i++) {
            for (int j = 0; j < dataStruct[i].second.length(); j++) {
                spec.append(qMakePair<Type, QString>(dataStruct[i].second[j].type,dataStruct[i].second[j].name));
            }
        }
        return spec;
    }

    quint32 visMsgLength;
    quint32 stratMsgLength;

private:
    NoChange noChange1;
    NoChange noChange4;
    SerializeInteger<quint16> uInt16;
    SerializeInteger<quint32> uInt32;
    SerializeInteger<quint64> uInt64;
    SerializeInteger<qint16> int16;
    SerializeInteger<qint32> int32;
    SerializeInteger<qint64> int64;
    FieldSerializer* serialize[10];

    //fields
    //ID 32 field 2 would be at Qpair list position lookUp[32], and lastDataStruct list position 2.
    QList<QPair<quint32, QList<LastDataStruct>>> dataStruct;
    QMutex dataBlocksMutex; //for id 0-10
    QMutex canMutex; //for id 10+
    quint32 lookUp[2048];

    bool newDataSinceLastCall;

    void initializeIds(); //the autogenerated function
    int loadInCSVSpec(int i, QString file);
    void showSpec();
    void restructureDatastructForDatablocks(int id);
};


#endif // CANSPEC_H
