#include "distancecalculator.h"

DistanceCalculator::DistanceCalculator() : dataPoints(1), first(true), lastSpeed(0),lastTimestamp(0), lastLat(0), lastLon(0), distanceLastTimestamp(0) {
    QString pathprefix = QDir(qApp->applicationDirPath()).absoluteFilePath("../../../");
    QFile inputFile(pathprefix+"routeData.txt");
    if (!inputFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open file: " << inputFile.errorString();
        return;
    }
    QTextStream in(&inputFile);
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList splitted = line.split(" ", QString::SkipEmptyParts);
        routeData.append(Row(splitted.at(0).toFloat(), splitted.at(1).toFloat(), splitted.at(2).toFloat(), splitted.at(3).toFloat()));
    }
    //qDebug() << "starting distance test";
    //distanceLastTimestamp = 11000;
}

float DistanceCalculator::getDistance(quint64 timestamp, float lat, float lon, float speed) {
    //if it's the first timestamp
    if (first) {
        initializeFirst(timestamp, lat, lon, speed);
        return distanceLastTimestamp;
    }
    float traveledEst = (speed + lastSpeed) * 0.5 * (timestamp-lastTimestamp) * 0.001;

    //if data has changed, we can make a better dist est
    if (lastLat != lat || lastLon != lon) {
        lastLat = lat;
        lastLon = lon;
        float newMK5Est = getDistanceThisTimestamp(traveledEst, lat, lon, false);
        //qDebug() << traveledEst;
        //qDebug() << newMK5Est;
        float distEst = getFilteredDistance(newMK5Est, traveledEst);
        updateVariables(timestamp, distEst, lat, lon, speed);
        return distEst;
        //TODO: merge this estimate with the distanceEst
    }

    updateVariables(timestamp, distanceLastTimestamp + traveledEst, lat, lon, speed);
    return distanceLastTimestamp;
}

float DistanceCalculator::getDistanceThisTimestamp(float distanceTraveled, float lat, float lon, bool checkAll) {
    qint32 index = (quint32) (distanceLastTimestamp + distanceTraveled)/100; //guess of where we are right now, rounded down
    if (index < 0) {
        index = 0;
    }
    else if (index >= routeData.length()) {
        index = routeData.length()-1;
    }
    //qDebug() << "distance this timestamp " << index << distanceTraveled << lat << lon << checkAll;
    float minSquaredDistance = getSquaredDistance(index, lat, lon);
    quint32 minIndex = 0;
    bool negativeBetterFound = true;
    bool positiveBetterFound = true;
    //find the closest poll
    for (int i = 1; positiveBetterFound || negativeBetterFound; i++) {
        if (negativeBetterFound && index-i >= 0) {
            //qDebug() << "step-" << index-i;
            if (getSquaredDistance(index-i, lat, lon) <= minSquaredDistance) {
                minSquaredDistance = getSquaredDistance(index-i, lat, lon);
                minIndex = index-i;
            }
            else if (!checkAll) {
                //qDebug() << "Stop negative";
                negativeBetterFound = false;
            }
        }
        else {
            //qDebug() << "Stop negative";
            negativeBetterFound = false;
        }
        if (positiveBetterFound && index+i < routeData.length()) {
            //qDebug() << "step+" << index+i;
            if (getSquaredDistance(index+i, lat, lon) <= minSquaredDistance) {
                minSquaredDistance = getSquaredDistance(index+i, lat, lon);
                minIndex = index+i;
            }
            else if (!checkAll) {
                //qDebug() << "Stop positive";
                positiveBetterFound = false;
            }
        }
        else {
            //qDebug() << "Stop positive";
            positiveBetterFound = false;
        }
    }
    quint32 other = minIndex;
    //find out which index lies closer, the one before the closet, or the one after
    if (minIndex == 0) {
        other = minIndex+1;
    }
    else if (minIndex + 1 >= routeData.length()) {
        other = minIndex-1;
    }
    else if (getSquaredDistance(minIndex-1, lat, lon) < getSquaredDistance(minIndex+1, lat, lon)) {
        other = minIndex-1;
    }
    else {
        other = minIndex+1;
    }

    //make sure that minIndex is the smaller index of the 2.
    if (minIndex > other) {
        quint32 temp = minIndex;
        minIndex = other;
        other = temp;
    }

    //just to make the formula's redable
    float x0 = lat;
    float y0 = lon;
    float x1 = routeData[minIndex].lat;
    float y1 = routeData[minIndex].lon;
    float x2 = routeData[other].lat;
    float y2 = routeData[other].lon;

    //some math, directly from wikipedia
    float distFromLine = qFabs((y2-y1)*x0 - (x2-x1)*y0 + x2*y1 - y2*x1)
                         /qSqrt((y2-y1)*(y2-y1) +(x2-x1)*(x2-x1));
    //some pythagoras
    float distFromMinIndex = qSqrt(getSquaredDistance(minIndex, lat, lon) - distFromLine*distFromLine);

    //now scale the distance in degrees to distance in metres
    distFromMinIndex *= 100/qSqrt((y2-y1)*(y2-y1) + (x2-x1)*(x2-x1));
    return routeData[minIndex].x + distFromMinIndex;
}

float DistanceCalculator::getSquaredDistance(quint32 index, float lat, float lon) {
    float latDiff = lat - routeData[index].lat;
    float lonDiff = lon - routeData[index].lon;
    return latDiff*latDiff + lonDiff*lonDiff;
}

float DistanceCalculator::initializeFirst(quint64 timestamp, float lat, float lon, float speed) {
    first = false;

    updateVariables(timestamp, getDistanceThisTimestamp(0, lat, lon, true), lat, lon, speed);
    return distanceLastTimestamp;
}

float DistanceCalculator::getFilteredDistance(float distEst, float distTraveled) {
    float ratio = 0.1;
    if (dataPoints < 8) {
        ratio = 1.0/(dataPoints+1.0);
        dataPoints++;
    }
    //qDebug() << "ratio " << ratio;
    //qDebug() << distEst;
    //qDebug() << distTraveled + distanceLastTimestamp;
    return ratio * distEst + (1.0 - ratio) * (distanceLastTimestamp + distTraveled);
}

void DistanceCalculator::updateVariables(quint64 timestamp, float dist, float lat, float lon, float speed) {
    lastLat = lat;
    lastLon = lon;
    distanceLastTimestamp = dist;
    lastTimestamp = timestamp;
    lastSpeed = speed;
}
