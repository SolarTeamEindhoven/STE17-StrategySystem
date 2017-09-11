#include <QApplication>
#include "datamanager.h"
#include <Qdebug>


//http://doc.qt.io/qt-4.8/qtendian.html TODO
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    DataManager dm;
    return a.exec();
}
