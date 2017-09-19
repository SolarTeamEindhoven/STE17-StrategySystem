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
    void setTimer2();

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
    QTimer timer2;
    quint32 id;
    quint32 size;
    quint32 type_id;
    bool readingType;
};

#endif // TCPCLIENT_H
