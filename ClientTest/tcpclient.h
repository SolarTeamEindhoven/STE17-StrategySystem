#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QHostAddress>
#include <QTcpSocket>
#include <QtCore>
#include <QtEndian>

class TcpClient : public QObject
{
     Q_OBJECT
public:
    TcpClient();

signals:
public slots:
    void newMessage();

    void write();
private:
    QTcpSocket socket;
};

#endif // TCPCLIENT_H
