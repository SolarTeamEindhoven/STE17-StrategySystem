#ifndef CANSPEC_H
#define CANSPEC_H

#include <QtCore>
#include <QtEndian>
#include <QTcpSocket>
#include "tcpprotocol_global.h"
//TODO autogenerate this

#define TOTALIDs 2

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
    UInt64 = 9
};

struct LastDataStruct {
    LastDataStruct(Type type, bool toVis, bool toStrat) : type(type), toVis(toVis), toStrat(toStrat), dataSize(0) {}

    //make sure we don't copy the mutex
    LastDataStruct(const LastDataStruct& copy) :
        type(copy.type),
        dataFields(copy.dataFields),
        toVis(copy.toVis),
        toStrat(copy.toStrat),
        dataSize(copy.dataSize) {}

    LastDataStruct& operator= (const LastDataStruct& data) {
        this->type = data.type;
        this->dataFields = data.dataFields;
        this->toVis = data.toVis;
        this->toStrat = data.toStrat;
        this->dataSize = data.dataSize;
        return *this;
    }
    Type type;
    QList<QByteArray> dataFields;
    QMutex mutex;
    bool toVis;
    bool toStrat;
    quint32 dataSize;
};

struct FieldSerializer {
public:
    virtual QByteArray fromLittle(QByteArray& data) = 0;
    virtual QByteArray toLittle(QByteArray& data) = 0;
    virtual const quint16 getSize() = 0;
};

struct NoChange : public FieldSerializer {
public:
    NoChange(quint16 size) : size(size) {}

    QByteArray fromLittle(QByteArray& data) override {
        return data;
    }
    QByteArray toLittle(QByteArray& data) override {
        return data;
    }
    const quint16 getSize() override {
        return size;
    }
private:
    const quint16 size;
};

template <typename T>
struct SerializeField : public FieldSerializer {
public:
    QByteArray fromLittle(QByteArray& data) override {
        QDataStream stream(&data, QIODevice::ReadWrite);
        T number;
        stream >> number;
        qFromLittleEndian(number);
        stream << number;
        return data;
    }

    QByteArray toLittle(QByteArray& data) override {
        QDataStream stream(&data, QIODevice::ReadWrite);
        T number;
        stream >> number;
        qToLittleEndian(number);
        stream << number;
        return data;
    }

    const quint16 getSize() override {
        return sizeof(T);
    }
};

class TCPPROTOCOLSHARED_EXPORT Serializer {
public:
    //this is a singleton class
    static Serializer* getInstance() {
        if (!initialized) {
            singleton = new Serializer();
            initialized = true;
        }
        else {
            return singleton;
        }
    }
    ~Serializer() {
        initialized = false;
    }

    QByteArray toLittleEndian(quint32 id, quint32 subId, QByteArray& data) {
        return toLittleEndian(dataStruct[id].second[subId].type, data);
    }

    QByteArray fromLittleEndian(quint32 id, quint32 subId, QByteArray& data) {
        return fromLittleEndian(dataStruct[id].second[subId].type, data);
    }

    QByteArray toLittleEndian(Type type, QByteArray& data) {
        return serialize[type]->toLittle(data);
    }

    QByteArray fromLittleEndian(Type type, QByteArray& data) {
        return serialize[type]->fromLittle(data);
    }
    QList<QPair<quint32, QList<LastDataStruct>>>* getData() {
        return &dataStruct;
    }

    void readAndChop(QTcpSocket* socket, quint32 id, quint32 dataSize, bool multipleLines) {
        if (id >= 10) {
            newDataSinceLastCall = true;
        }
        QList<LastDataStruct>& fieldList = dataStruct[id].second;

        //read the first line
        for (int i = 0; i < fieldList.size(); i++) {
            fieldList[i].mutex.lock(); //close the mutex
            fieldList[i].dataFields.clear(); //clear the line

            quint16 thisSize = serialize[fieldList[i].type]->getSize();
            fieldList[i].dataFields.append(socket->read(thisSize)); //append datafield to list
            fieldList[i].dataSize = thisSize;
            dataSize -= thisSize;
        }

        //if there are multiple lines in this datablock, read the rest of the lines, else throw away the rest of the size
        if (multipleLines) {
            while (dataSize > 0) {
                for (int i = 0; i < fieldList.size(); i++) {
                    quint16 thisSize = serialize[fieldList[i].type]->getSize();
                    fieldList[i].dataFields.append(socket->read(thisSize)); //append datafield to list
                    fieldList[i].dataSize += thisSize;
                    dataSize -= thisSize;
                }
            }
        }
        //open all mutexes again
        for (int i = 0; i < fieldList.size(); i++) {
            fieldList[i].mutex.unlock();
        }
    }

    QByteArray getLastData(quint32 id) {
        QList<LastDataStruct>& lastData = dataStruct[lookUp[id]].second;
        QByteArray data;
        for (int j = 0; lastData.last().dataFields.size(); j++) { //if there are multiple lines
            for (int i = 0; i < lastData.size(); i++) {
                lastData[i].mutex.lock();
                data.append(lastData[i].dataFields.at(j));
                lastData[i].mutex.unlock();
            }
        }
        return data;
    }

    QByteArray getLastData(quint32 id, quint32 subId) {
         QList<LastDataStruct>& lastData = dataStruct[lookUp[id]].second;
         QByteArray data;
         lastData[subId].mutex.lock();
         for (int j = 0; lastData[subId].dataFields.size(); j++) { //if there are multiple lines in this slot
            data.append(lastData[subId].dataFields.at(j));
         }
         lastData[subId].mutex.unlock();
         return data;
    }

    QByteArray getStratCANData() {
         QByteArray data;
         for (int i = 0; i < dataStruct.size(); i++) {
            if (dataStruct[i].first >= 10) {
                for (int j = 0; j < dataStruct[i].second.size(); j++) {
                    LastDataStruct* str = &dataStruct[i].second[j];
                    if (str->toStrat) {
                        str->mutex.lock();
                        data.append(str->dataFields[0]);
                        str->mutex.unlock();
                    }
                }
            }
        }
        return data;
    }

    QByteArray getVisData() {
         QByteArray data;
         for (int i = 0; i < dataStruct.size(); i++) {
            if (dataStruct[i].first >= 10) {
                for (int j = 0; j < dataStruct[i].second.size(); j++) {
                    LastDataStruct* str = &dataStruct[i].second[j];
                    if (str->toVis) {
                        str->mutex.lock();
                        data.append(str->dataFields[0]);
                        str->mutex.unlock();
                    }
                }
            }
        }
        return data;
    }
    QPair<quint32, QByteArray> getField(quint32 id) {
        QByteArray data;
        quint32 size = 0;
        QList<LastDataStruct>& str = dataStruct[id].second;
        for (quint32 j = 0; j < str.size(); j++) {
            str[j].mutex.lock();
        }
        for (quint32 i = 0; i < str[0].dataFields.size(); i++) { //loop over the rows
            for (quint32 j = 0; j < str.size(); j++) { //for each row, construct this row
                data += str[j].dataFields[i];
            }
        }
        for (quint32 j = 0; j < str.size(); j++) {
            size += str[j].dataSize; //loop over the size
            str[j].mutex.unlock();
        }
        return qMakePair<quint32, QByteArray>(size, data);
    }

    bool checkNewData() {
        if (newDataSinceLastCall) {
            newDataSinceLastCall = false;
            return true;
        }
        return false;
    }



private:
    NoChange noChange1;
    NoChange noChange4;
    SerializeField<quint16> uInt16;
    SerializeField<quint32> uInt32;
    SerializeField<quint64> uInt64;
    SerializeField<qint16> int16;
    SerializeField<qint32> int32;
    SerializeField<qint64> int64;
    FieldSerializer* serialize[10];

    //fields
    //ID 32 field 2 would be at Qpair list position lookUp[32], and lastDataStruct list position 2.
    QList<QPair<quint32, QList<LastDataStruct>>> dataStruct;
    quint32 lookUp[2048];

    bool newDataSinceLastCall;
    //singleton fields
    static bool initialized;
    static Serializer* singleton;

    void initializeIds(); //the autogenerated function

    Serializer() : noChange1(1), noChange4(4),
                   serialize{&noChange4, &noChange1, &noChange1, &uInt16, &uInt32, &uInt64, &noChange1, &int16, &int32, &int64},
                   lookUp{ 0 }, newDataSinceLastCall(false)
    {
        initializeIds(); //this is autogenerated
    }
};


#endif // CANSPEC_H
