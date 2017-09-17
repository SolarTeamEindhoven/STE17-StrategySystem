#ifndef VISTIMERTHREAD_H
#define VISTIMERTHREAD_H

#include <QThread>
#include <Qtimer>
#include <QDebug>
class VisTimerThread : public QThread
{
    Q_OBJECT
public:
    VisTimerThread(QTimer* timer);
    ~VisTimerThread();

private:
    void run() override;
    QTimer* timer;
};

#endif // VISTIMERTHREAD_H
