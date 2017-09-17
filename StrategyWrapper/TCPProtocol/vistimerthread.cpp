#include "vistimerthread.h"

//yep, timers have to be started in their own threads,
VisTimerThread::VisTimerThread(QTimer *timer) : timer(timer)
{
}

VisTimerThread::~VisTimerThread() {}

void VisTimerThread::run() {
    timer->start();
    //I want sending the rows to be of top priority above writing large data blocks,
    //else the timer callbacks and sending is going to stagnate.
    setPriority(QThread::TimeCriticalPriority);
    this->exec();
}

