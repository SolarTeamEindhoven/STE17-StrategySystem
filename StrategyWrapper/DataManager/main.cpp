#include <QApplication>
#include "datamanager.h"
#include <Qdebug>


//http://doc.qt.io/qt-4.8/qtendian.html TODO
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    qDebug() << "test";
    DataManager dm;
    qDebug() << "test2";
    dm.timerCallBack();
    return a.exec();
}
