#ifndef DISTANCECALCULATOR_H
#define DISTANCECALCULATOR_H

#include <QtCore>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QtMath>

class DistanceCalculator
{
public:
    DistanceCalculator();
    float getDistance(quint64 timestamp, float lat, float lon, float speed);

private:
    struct Row {
        Row(float id, float lat, float lon, float x) : id(id), lat(lat), lon(lon), x(x) {}
        float id;
        float lat;
        float lon;
        float x;
    };

    QList<Row> routeData;
    quint16 dataPoints;
    bool first;

    quint64 lastTimestamp;
    float distanceLastTimestamp;
    float lastSpeed;
    float lastLat;
    float lastLon;

    float getDistanceThisTimestamp(float distanceTraveled, float lat, float lon, bool checkAll);
    float combineDistances(quint64 thisTimestamp, float distance);
    float initializeFirst(quint64 timestamp, float lat, float lon, float speed);
    float getSquaredDistance(quint32 index, float lat, float lon);
    float getFilteredDistance(float distEst, float distTraveled);
    void updateVariables(quint64 timestamp, float dist, float lat, float lon, float speed);
};

#endif // DISTANCECALCULATOR_H
