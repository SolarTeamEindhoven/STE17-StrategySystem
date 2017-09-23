#ifndef MK5CLIENT_H
#define MK5CLIENT_H

#include <QThread>
#include <QTcpSocket>
#include <QHostAddress>
#include "sockethandler.h"

class MK5Client : public QThread
{
public:
    MK5Client(DataManager* dm);
public slots:
    void disconnectSocket();
private:
    QTcpSocket socket;
    SocketHandler handler;

    void run() override;
};

#endif // MK5CLIENT_H
