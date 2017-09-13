#include "dbhandler.h"

DBHandler::DBHandler(QList<QPair<Type, QString>>& canSpec) :
canSpec(canSpec) {
    db = QSqlDatabase::addDatabase("QMySQL","Connection");
    db.setHostName("DB Handler STE");
    db.setDatabaseName("STE Strategy Database Test");
    db.setUserName("Data Manager");
    db.setPassword("beterlompdantweede");
    db.open();
    create();
}

void DBHandler::create() {
    QString canCreateQuery;
    canCreateQuery.append("CREATE TABLE IF NOT EXISTS canmessages ( ");
    canCreateQuery.append("timestamp TIMESTAMP NOT NULL,");
    canCreateQuery.append("distance FLOAT NOT NULL ");
    canCreateQuery.append(createFields(canSpec));
    canCreateQuery.append(");");
    db.exec(canCreateQuery);

    QString wfsCreateQuery;
    wfsCreateQuery.append("CREATE TABLE IF NOT EXISTS masterForecast ( ");
    wfsCreateQuery.append("id INT UNSIGNED NOT NULL ");
    wfsCreateQuery.append(createFields(wfsSpec));
    wfsCreateQuery.append(");");
    db.exec(wfsCreateQuery);

    QString stsCreateQuery;
    stsCreateQuery.append("CREATE TABLE IF NOT EXISTS sts ( ");
    stsCreateQuery.append(createFields(stsSpec));
    stsCreateQuery.append(");");
    db.exec(stsCreateQuery);

    QString ltsCreateQuery;
    ltsCreateQuery.append("CREATE TABLE IF NOT EXISTS lts ( ");
    ltsCreateQuery.append(createFields(ltsSpec));
    ltsCreateQuery.append(");");
    db.exec(ltsCreateQuery);

    QString paramCreateQuery;
    paramCreateQuery.append("CREATE TABLE IF NOT EXISTS lts ( ");
    paramCreateQuery.append(createFields(paramSpec));
    paramCreateQuery.append(");");
    db.exec(paramCreateQuery);
}

QString DBHandler::createFields(QList<QPair<Type, QString>>& spec) {
    QString createQuery;
    for (int i = 10; i < spec.length(); i++) {
        createQuery.append(", ");
        createQuery.append(spec[i].second); //add name
        switch(spec[i].first) { //add type
        case UInt8 :
            createQuery.append(" TINYINT UNSIGNED");
            break;
        case Float :
            createQuery.append(" FLOAT");
            break;
        case Other :
            createQuery.append(" TINYINT UNSIGNED");
            break;
        case UInt16 :
            createQuery.append(" SMALLINT UNSIGNED");
            break;
        case UInt32 :
            createQuery.append(" INT UNSIGNED");
            break;
        case UInt64 :
            qDebug() << "Warning, uint64 is being interpreted as int64.";
            createQuery.append(" INT");
            break;
        case Int8 :
            createQuery.append(" TINYINT");
            break;
        case Int16 :
            createQuery.append(" SMALLINT");
            break;
        case Int32 :
            createQuery.append(" INT");
            break;
        case Int64 :
            createQuery.append(" BIGINT");
            break;
        default :
            qDebug() << "Warning, type is not being recognized.";
            break;
        }
        createQuery.append(" NOT NULL");
    }
    return createQuery;
}
