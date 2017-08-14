#ifndef CLIENT_H
#define CLIENT_H


#include <QtCore>
#include <QtNetwork>


QByteArray intToArray(qint32 source);

class Client : public QObject
{
    Q_OBJECT
public:
    explicit Client(QObject *parent = 0);

public slots:
    bool connectToHost(QString host);
    bool writeData(QByteArray data);

private:
    QTcpSocket *socket;
};




#endif // CLIENT_H
