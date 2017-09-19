#include "dbhandler.h"
#include <QSqlError>

DBHandler::DBHandler() : db(QSqlDatabase::addDatabase("QMYSQL","Connection")) {
    db.setHostName("localhost");//("DB Handler STE");
    db.setPort(3306);
    db.setDatabaseName("stestrategydatabasetest");
    db.setUserName("DataManager");
    db.setPassword("beterlompdantweede");
    qInfo() << db.open();
    qInfo() << db.lastError();
    qInfo() << db.isValid() << " <- valid | open --> " << db.isOpen();
}

void DBHandler::setSpec(QList<QPair<Type, QString>>& canSpec,
                         QList<QPair<Type, QString>>& wfsSpec,
                         QList<QPair<Type, QString>>& stsSpec,
                         QList<QPair<Type, QString>>& ltsSpec,
                         QList<QPair<Type, QString>>& paramSpec) {
    this->canSpec = canSpec;
    this->wfsSpec = wfsSpec;
    this->stsSpec = stsSpec;
    this->ltsSpec = ltsSpec;
    this->paramSpec = paramSpec;
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
            qDebug() << "Warning, uint64 is being interpreted as timestamp.";
            createQuery.append(" TIMESTAMP");
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
        createQuery.append(" NOT NULL"); //TODO do I want this?
    }
    return createQuery;
}
