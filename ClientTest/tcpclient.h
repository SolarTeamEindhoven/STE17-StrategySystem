#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QHostAddress>
#include <QTcpSocket>
#include <QtCore>
#include <QtEndian>
#include <QTime>
#include <QTimer>

class TcpClient : public QObject
{
     Q_OBJECT
public:
    TcpClient(quint32 number);
    void setTimer();

signals:
public slots:
    void newMessage();

    void identify(quint32 type_id);
    void writeLargeData();
    void timerCheckout();
private:
    quint32 number;
    QTcpSocket socket;
    QTimer timer;
    quint32 id;
};

#endif // TCPCLIENT_H
