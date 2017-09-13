#ifndef DBHANDLER_H
#define DBHANDLER_H

#include <QtCore>
#include <QtSql/QtSql>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include "serializer.h"

#define CREATENEWDB true


class DBHandler
{
public:
    DBHandler(QList<QPair<Type, QString>>& canSpec);
private:
    void create();
    QSqlDatabase db;
    QList<QPair<Type, QString>> canSpec;
    QList<QPair<Type, QString>> wfsSpec;
    QList<QPair<Type, QString>> stsSpec;
    QList<QPair<Type, QString>> ltsSpec;
    QList<QPair<Type, QString>> paramSpec;
    QString createFields(QList<QPair<Type, QString>>& spec);
};

#endif // DBHANDLER_H
