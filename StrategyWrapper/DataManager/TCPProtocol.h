#include <QObject>
#include <QtEndian>
#include <QtCore>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QMutex>

struct MyDatapoint {
	QDateTime time;
        QVariant data;
};
using MyDatablock = QList<MyDatapoint>;

struct MyQuery {
        MyStreamId id;
        QDateTime time1;
        QDateTime time2;
};

struct MyStreamId {
public:
       MyStreamID(quint16 id) : id(id) {}
       quint16 id;
};

class MySocketWriteHandler {
public:
        MySocketWriteHandler(const QTcpSocket* origin)
		: origin(origin)
        {
            timer->setInterval(200); //if no new message has sent in 200ms, flush the socket
            connect(timer, SIGNAL(timeout()), SLOT(handleTimout()));
            connect(this->origin, SIGNAL(bytesWritten(qint64)), SLOT(bytesWritten(quint64));

        }

        ~MySocketWriteHandler() {
	}

        void sendSubscribeMessage(MyStreamId id, bool subscribe) {
            QByteArray data;
            //TODO: implement this
            origin.write(data);
        }

        void sendQuery(MyStreamId id, QDateTime time1, QDateTime time2) {
            QByteArray data;
            //TODO: implement this
            origin.write(data);
        }

        void sendData(MyStreamId id, MyDatapoint data) {
            QList<MyDatapoint> dataBlock;
            dataBlock.append(data);
            sendData (id, datablock);
        }

        void sendData(MyStreamId id, const MyDatablock dataBlock) {
            QByteArray data;
            QDataStream dataStream(&data, QIODevice::ReadWrite);
            //I won't use foreach: in this way the data stays in the same order
            for (int i = 0; i < dataBlock.size(); i++) {
                dataStream << qToLittleEndian(dataBlock.at(i).time);
                //todo serialize the qbytearray and send it (serealizer still needs to be adjusted)

            }
            origin.write(data);
        }
        bool compareSocket(const QTcpSocket* compare) {
            return compare == origin;
        }

public slots:
        void handleTimeout() {
            //if this slot is signalled, no bytes have been written for more than 200ms
            //TODO: uitzoeken of dit gecalled wordt als de socket data verstuurt of als ik data naar de socket schrijf
            origin->flush();
        }
        void bytesWritten(quint64) {
            timer->start(); //restart the timer
        }

private:
        const QTcpSocket* origin;
        QTimer* timer; //used to time wether we need to flush or not
};

enum class MyFieldType {
	Bool,
        Int16,
        Uint16,
	Int32,
	UInt32,
	Int64,
	UInt64,
	Float,
	Double,
        Other,

	Int16List,
	// TODO: Extend...
};

class MyAbstractDeSerializer {
public:
	virtual bool operator() (QIODevice&) = 0;
	virtual void reset() = 0;
	QVariant value;
};

template<typename T>
class MySimpleDeSerializer final : public MyAbstractDeSerializer {
public:
	bool operator() (QIODevice& dev) override {
		if(dev.bytesAvailable() < sizeof(T))
			return false;

		union {
			T n;
			char raw[sizeof(T)];
		} result;
		dev.read(result.raw, sizeof(T));
		value = QVariant::fromValue(letoh(result.n));
		return true;
	}
        void reset() override {}; //TODO: implement this
};

class MyInt16ListDeSerializer final : public MyAbstractDeSerializer {
public:
	bool operator() (QIODevice& dev) override {
		while(process(dev));
		return state == State::Done;
	}
	void reset() override {
		state = State::Length;
	};

	static QByteArray serialize(const QVariant&) {
		// TODO: Mag je zelf doen
	}

private:
	uint32_t count;
	enum class State {
		Length,
		Entries,
		Done
	} state;

	bool process(QIODevice& dev) {
		switch(state) {
		case State::Length:
			return readLength(dev);
		case State::Entries:
			return readEntry(dev);
		case State::Done:
			return false;
		}
	}

	bool readLength(QIODevice& dev) {
		if(dev.bytesAvailable() < sizeof(uint32_t))
			return false;

		union {
			uint32_t n;
			char raw[sizeof(uint32_t)];
		} length;
		dev.read(length.raw, sizeof(uint32_t));
		count = letoh(length);

		value = QVariant::fromValue(QList<int>());
		state = State::Entries;

		return true;
	}

	bool readLength(QIODevice& dev) {
		if(dev.bytesAvailable() < sizeof(uint16_t))
			return false;

		union {
			uint16_t n;
			char raw[sizeof(uint16_t)];
		} length;
		dev.read(length.raw, sizeof(uint16_t));
		value.value<QList<int>>() << letoh(length);

		if(--count == 0)
			state == State::Done;

		return true;
	}
};

class MyStreamDataSerializer {
public:
        QList<MyFieldType> getStreamType(MyStreamId id) {
            //hopefully the optimizer manages to optimize this TODO: eens bij steve zeuren
            QVector<MyFieldType> returnlist;
            switch(id.id) {
                case 1:
                    returnlist.append(MyFieldType::Bool);
                    returnlist.append(MyFieldType::Float);
                    return returnlist;
                case 2:
                    returnlist.append(MyFieldType::Int16);
                    returnlist.append(MyFieldType::Int16);
                    return returnlist;
            }

		// TODO:
                //  Auto generate this
	}
	bool isValidStreamId(uint32_t id) {
             switch(id.id) {
                case 1:
                    return true;
                case 2:
                    return true;
                default:
                    return false;
             }
             return false;
		// TODO:
                // Auto generate this
	}

        uint32_t getSerializedLength(MyFieldType type) {
		switch(type) {
                case MyFieldType::Bool:
			return sizeof(bool);
                case MyFieldType::Int32:
			return sizeof(int);
                case MyFieldType::UInt32:
			return sizeof(unsigned int);
                // ... TODO, deze lijst maar eens aanvullen
		}
	}

        //TODO: adjust for QVector<MyFieldType>
        QByteArray serialize(MyFieldType type, const QVariant& v) {
		switch(type) {
                case MyFieldType::Bool:
			return serializeSingleType(QVariant::value<bool>());
                case MyFieldType::Int32:
			return serializeSingleType(QVariant::value<int>());
                case MyFieldType::UInt32:
			return serializeSingleType(QVariant::value<unsigned int>());
		// ...
                case MyFieldType::Int16List:
			return MyInt16ListDeSerializer::serialize(value);
                // ... TODO
		}
	}
        std::unique_ptr<MyAbstractDeSerializer> deSerialize(MyFieldType type, const QByteArray& data) {
		switch(type) {
                case MyFieldType::Bool:
			return std::make_unique<MySimpleDeSerializer<bool>>();
                case MyFieldType::Int32:
			return std::make_unique<MySimpleDeSerializer<int>>();
                case MyFieldType::UInt32:
			return std::make_unique<MySimpleDeSerializer<unsigned int>>();
		// ...
                case MyFieldType::Int16List:
			return std::make_unique<MyInt16ListDeSerializer>();
                // ... TODO
		}
	}

	static streamDataSerializer& instance() {
		static MyStreamDataSerializer streamDataSerializer;
		return streamDataSerializer;
	}

private:
	MyStreamDataSerializer();

	template<typename T>
	static QByteArray serializeSingleType(T v) {
		T d = htole(v);
		return QByteArray( reinterpret_cast<char*>(&d), sizeof(T) );
	}
	template<typename T>
	static T deSerializeSingleType(const QByteArray& data) {
		union {
			T n;
			char raw[sizeof(T)];
		} result;
		std::memcpy(result.raw, data.constData(), sizeof(T));
		return letoh(result.n);
	}
};

class MySocketReadHandler {
public:
        MySocketReadHandler(QTcpSocket* socket) {
            connect(socket, SIGNAL(readyRead()), SLOT(handleTcpData));
        }

signals:
        void subscribeRequest(MyStreamId, const QTcpSocket*);
        void unsubscribeRequest(MyStreamId, const QTcpSocket*);
        void receivedDataPoint(MyStreamId, const MyDatablock&);
        void newDataMessage(MyStreamId);
        void newQuery(MyQuery);

public slots:
        void handleTcpData() {
                QTcpSocket* origin = qobject_cast<>(sender());
                if(origin == nullptr)
                        return;
                while(handleTcpData(*origin));
        }

private:
	enum class State {
		MessageID,
		SubscribeMessage,
		QueryMessage,
		NewDataMessage,
                NewDataMessageEntryDateTime,
                NewDataMessageEntryData,
	} readState;


	bool handleTcpData(QTcpSocket& origin) {
		switch(readState) {
		case State::MessageID:
			return readMessageId(origin);
		case State::SubscribeMessage:
			return readSubscribeMessage(origin);
		case State::QueryMessage:
			return readQueryMessage(origin);
		case State::NewDataMessage:
			return readNewDataMessage(origin);
		case State::NewDataMessageEntryDateTime:
			return readNewDataMessageDateTime(origin);
		case State::NewDataMessageEntryData:
			return readNewDataMessageData(origin);
		}
	}

	bool readMessageId(QTcpSocket& origin) {
		if(origin.bytesAvailable() < sizeof(uint8_t))
			return false;

		uint8_t id;
		origin.read(&id, sizeof(uint8_t));

		switch(id) {
		case 0:
			readState = State::SubscribeMessage;
		case 1:
			readState = State::QueryMessage;
		case 2:
			readState = State::NewDataMessage;
		default:
			// Protocol error
		}

		return true;
	}

	bool readSubscribeMessage(QTcpSocket& origin) {
                if(origin.bytesAvailable() < (sizeof(uint8_t) + sizeof(uint16_t)))
			return false;
                MyStreamId id = readStreamId(origin);

                //read subscribtion
                uint8_t subscribe;
                origin.read(&subscribe, sizeof(uint8_t));

		if(subscribe == 0)
                        emit unsubscribeRequest(id, origin);
		else
                        emit subscribeRequest(id, origin);

		readState = State::MessageID;
		return true;
	}

	bool readQueryMessage(QTcpSocket& origin) {
            if(origin.bytesAvailable() < 18 * sizeof(int8_t)) //message is 18 bytes (+ the 1 byte that has already been looked at) long
                    return false;

            MyQuery query;
            query.id = readStreamId(origin);

            union {
                    int64_t n;
                    char raw[sizeof(int64_t)];
            } timestamp1, timestamp2;
            origin.read(timestamp1.raw, sizeof(int64_t));
            origin.read(timestamp2.raw, sizeof(int64_t));
            query.time1.setMSecsSinceEpoch(letoh(timestamp1.n));
            query.time2.setMSecsSinceEpoch(letoh(timestamp2.n));
            emit newQuery(query);
	}
	
	StreamID newEntryStreamId;
	std::unique_ptr<MyAbstractDeSerializer> deSerializer;
	uint32_t newEntryCount;
        MyDatablock dataBlock;

	bool readNewDataMessage(QTcpSocket& origin) {
		if(origin.bytesAvailable() < (sizeof(uint32_t) + sizeof(uint32_t)))
			return false;

                MyStreamId id = readStreamId(origin);

		union {
			uint32_t n;
			char raw[sizeof(uint32_t)];
                } length;
		origin.read(length.raw, sizeof(uint32_t));

		if(!MyStreamDataSerializer::instance().isValidStreamId(letoh(newDataStreamId)))
			; // Protocol error

                newEntryStreamId = id;
		newEntryCount = letoh(length.n);
                dataBlock.clear();
                MyFieldType type = MyStreamDataSerializer::instance().getStreamType(newEntryStreamId);
		deSerializer = MyStreamDataSerializer::instance().deSerialize(type);
		readState = State::NewDataMessageEntriesDateTime;
		return true;
	}

	QDateTime newDataEntryDateTime;
	bool readNewDataMessageEntryDateTime(QTcpSocket& origin) {
		if(origin.bytesAvailable() < sizeof(int64_t))
			return false;

		union {
			int64_t n;
			char raw[sizeof(int64_t)];
		} timestamp;
		origin.read(timestamp.raw, sizeof(int64_t));
                newDataEntryDateTime.setMSecsSinceEpoch(letoh(timestamp.n));

		readState = State::NewDataMessageEntriesData;
		return true;
	}

	bool readNewDataMessageEntryData(QTcpSocket& origin) {
		if(deSerializer(origin)) {
                        MyDatapoint dp {
				newDataEntryDateTime,
				deSerializer.value
			};

                        dataBlock.append(dp);

			if(--newEntryCount == 0)
				readState = State::MessageID;
                                emit receivedDataPoint(newEntryStreamId, dataBlock);
                                dataBlock.clear();
			else {
				readState = State::NewDataMessageEntriesDateTime;
				deSerializer.reset();
			}

			return true;
		}

		return false;
	}

        //some helper functions

        MyStreamId readStreamId(QTcpSocket& origin) {
            union {
                    uint16_t n;
                    char raw[sizeof(uint16_t)];
            } num;
            origin.read(num.raw, sizeof(uint16_t));
            num.n = letoh(num.n);
            if(!MyStreamDataSerializer::instance().isValidStreamId(num.n))
                    ; // Protocol error

            return MyStreamId(static_cast<quint16>(num.n));
        }
};

class MyProtocolServerImpl {
public:
    MyProtocolClientImpl() {
        server = new QTcpServer(this);
        connect(server, SIGNAL(newConnection()), SLOT(newConnection()));
    }

public slots:
        void updateData(MyStreamId, MyDatapoint);
        void newConnection() {
            while (server->hasPendingConnections())
            {
                QTcpSocket *socket = server->nextPendingConnection();
                clientlist.insert(socket, qmakepair(MySocketReadHandler(), MySocketWriteHandler(socket)));

                MySocketReadHandler* thisHandler = &clientlist[socket].first; //temporary pointer
                connect(socket,SIGNAL(disconnected()), SLOT(disconnected())); //via this way, we can end the stream
                connect(thisHandler, SIGNAL(receivedDataPoint(MyStreamId, const MyDatablock&)), SLOT(handleNewdata(MyStreamId, const MyDatablock&));
                connect(thisHandler, SIGNAL(subscribeRequest(MyStreamId, const QTcpSocket*)), SLOT(handleSubscription(MyStreamId, const QTcpSocket*)));
                connect(thisHandler, SIGNAL(subscribeRequest(MyStreamId, const QTcpSocket*)), SLOT(handleUnsubscription(MyStreamId, const QTcpSocket*)));
                connect(thisHandler, SIGNAL(newQuery(MyQuery)), SLOT(handleQuery(MyQuery)));
            }
        }

        void disconnected() {
            QTcpSocket* origin = qobject_cast<>(sender());
            //remove a socket from the subscriptionlist (this might need some optimization)
            QList<quint16> keys = subscriptionlist.keys();
            for (quint16 key : keys) {
                QVector<QTcpSocket*> temp = subscriptionlist[key];
                for (int i = 0; i < temp.size(); i++) {
                    if (temp[i]->compareSocket(origin)) {
                        temp.remove(i);
                    }
                }
            }
            //remove the handler from the list
            clientlist.remove(origin);
        }

        void handleNewdata(MyStreamId id , const MyDatablock& db) {
            for (MySocketWriteHandler* socketHandler : subscriptionlist[id]) {
                socketHandler->sendData(id, db);
            }
            QPair<MyDatablock, QMutex> pair = &lastReceivedDatablock[id];
            pair.second.lock();
            pair.first = db;
            pair.second.unlock();
            //TODO: write this to the database
        }

        void handleSubscription(MyStreamId id, const QTcpSocket* socket) {
            subscriptionlist[id].append(socket);
            MySocketWriteHandler writeandler =

            QPair<MyDatablock, QMutex> pair = &lastReceivedDatablock[id];
            pair.second.lock();
            pair.first = db;
            pair.second.unlock();

        }

        void handleUnsubscription(MyStreamId id) {
            subscriptionlist[id].removeOne(socket);
        }

        void handleQuery(MyQuery query, const QTcpSocket*) {
            //TODO: make the database query, recieve query, build a new message and send it
        }

signals:
        void receivedData(MyStreamId, MyDatapoint);
        void receivedQuery(MyQuery, MySocketWriteHandler);
private:
        QTcpServer* server;
        QHash<QTcpSocket*, QPair<MySocketReadHandler, MySocketWriteHandler>> clientlist; //the pairing with QTcpSocket is needed in order to find the mysocketreadhandler based on the slot
        QHash<quint16, QVector<MySocketWriteHandler*>> subscriptionlist;
        QHash<quint16, QPair<MyDatablock, QMutex>> lastReceivedDatablock;
};

class MyProtocolClientImpl {
public:
	void subscribe(MyStreamId);
	void unsubscribe(MyStreamId);
	void request(MyQuery);

public slots:
        void updateData(MyStreamId, MyDatapoint);
	
signals:
        void receivedData(MyStreamId, MyDatapoint);
};
