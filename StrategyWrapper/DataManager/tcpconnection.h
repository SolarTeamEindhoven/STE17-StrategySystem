#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H

#include <QtCore>
#include <QtNetwork>
#include <QThread>

class TCPConnection : public QThread
{
public:
    TCPConnection();
    QTcpSocket *socket;
    void run() override;

private:

};

#endif // TCPCONNECTION_H
