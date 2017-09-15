#ifndef DBHANDLER_H
#define DBHANDLER_H

#include <QtCore>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include "serializer.h"
#include "tcpprotocol_global.h"

#define CREATENEWDB true


class TCPPROTOCOLSHARED_EXPORT DBHandler : public QObject
{
    Q_OBJECT
public:
    DBHandler();
    void setSpec(QList<QPair<Type, QString>>& canSpec,
                             QList<QPair<Type, QString>>& wfsSpec,
                             QList<QPair<Type, QString>>& stsSpec,
                             QList<QPair<Type, QString>>& ltsSpec,
                             QList<QPair<Type, QString>>& paramSpec);
    void create();
private:
    QSqlDatabase db;
    QList<QPair<Type, QString>> canSpec;
    QList<QPair<Type, QString>> wfsSpec;
    QList<QPair<Type, QString>> stsSpec;
    QList<QPair<Type, QString>> ltsSpec;
    QList<QPair<Type, QString>> paramSpec;
    QString createFields(QList<QPair<Type, QString>>& spec);
};

#endif // DBHANDLER_H
