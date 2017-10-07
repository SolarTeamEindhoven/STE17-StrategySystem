#include "distancecalculator.h"

DistanceCalculator::DistanceCalculator() : dataPoints(1), first(true), lastSpeed(0),lastTimestamp(0), lastLat(0), lastLon(0), distanceLastTimestamp(0) {
    QString pathprefix = QDir(qApp->applicationDirPath()).absoluteFilePath("../../../");
    QFile inputFile(pathprefix+"RouteData.txt");
    if (!inputFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open file: " << inputFile.errorString();
        return;
    }
    distanceLastTimestamp = 0000;
    distMK5 = distanceLastTimestamp;
    distSpeed = distanceLastTimestamp;
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
    if (lat == 0.0 && lon == 0.0) { //initializing
        return distanceLastTimestamp;
    }
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
        distMK5 = newMK5Est;
        distSpeed += traveledEst;
        qDebug() << "distMK5" <<distMK5;
        qDebug() << "distSpeed" << distSpeed;
        //qDebug() << "last + speed " << distanceLastTimestamp + traveledEst;
        //qDebug() << "last " << distanceLastTimestamp;
        //qDebug() << "New MK5 " << newMK5Est;
        float distEst = getFilteredDistance(newMK5Est, traveledEst);
        updateVariables(timestamp, distEst, lat, lon, speed);
        return distEst;
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
    //qDebug() << "index " << index;
    //qDebug() << "distance this timestamp " << index << distanceTraveled << lat << lon << checkAll;
    float minSquaredDistance = getSquaredDistance(index, lat, lon);
    quint32 minIndex = index;
    bool negativeBetterFound = true; //todo change this
    bool positiveBetterFound = true;
    //find the closest poll
    for (int i = 1; (positiveBetterFound || negativeBetterFound) && i < 200; i++) {
        if (negativeBetterFound && index-i >= 0) {
            //qDebug() << "step-" << index-i;
            if (getSquaredDistance(index-i, lat, lon) <= minSquaredDistance) {
                minSquaredDistance = getSquaredDistance(index-i, lat, lon);
                //qDebug() << index-i <<  minSquaredDistance;
                minIndex = index-i;
            }
            else if (!checkAll && negativeBetterFound) {
                //qDebug() << "Stop negative" << index-i;
                negativeBetterFound = false;
            }
            else {
                //qDebug() << index+i;
            }
        }
        else if (negativeBetterFound) {
            //qDebug() << "Stop negative" << index-i;
            negativeBetterFound = false;
        }
        if (positiveBetterFound && index+i < routeData.length()) {
            //qDebug() << "step+" << index+i;
            if (getSquaredDistance(index+i, lat, lon) <= minSquaredDistance) {
                minSquaredDistance = getSquaredDistance(index+i, lat, lon);
                //qDebug() << index+i <<  minSquaredDistance;
                minIndex = index+i;
            }
            else if (!checkAll && positiveBetterFound) {
                //qDebug() << index+i;
                //qDebug() << "Stop positive" << index+i;
                positiveBetterFound = false;
            }
            else {
                //qDebug() << index-i;
            }
        }
        else if (positiveBetterFound) {
            //qDebug() << "Stop positive" + index+i;;
            positiveBetterFound = false;
        }
    }
    qDebug() << "minIndex" << minIndex;
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
    //qDebug() << "min index" << minIndex;

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

    //qDebug() << "dist from line " << distFromLine;
    //some pythagoras
    float distFromMinIndex = qSqrt(getSquaredDistance(minIndex, lat, lon) - distFromLine*distFromLine);

    //qDebug() << "dist from minIndex" << distFromMinIndex;
    //now scale the distance in degrees to distance in metres
    distFromMinIndex *= 100/qSqrt((y2-y1)*(y2-y1) + (x2-x1)*(x2-x1));
    qDebug() << "dist from minIndex scaled" << distFromMinIndex;
    //qDebug() << "mk5 dist" << routeData[minIndex].x + distFromMinIndex;
    //qDebug() << "routedata point" << routeData[minIndex].x;
    return routeData[minIndex].x + distFromMinIndex;
}

float DistanceCalculator::getSquaredDistance(quint32 index, float lat, float lon) {
    float latDiff = lat - routeData[index].lat;
    float lonDiff = lon - routeData[index].lon;
    return latDiff*latDiff + lonDiff*lonDiff;
}

float DistanceCalculator::initializeFirst(quint64 timestamp, float lat, float lon, float speed) {
    //qDebug() << "initializefirst";
    first = false;

    updateVariables(timestamp, getDistanceThisTimestamp(0, lat, lon, true), lat, lon, speed);
    return distanceLastTimestamp;
}

float DistanceCalculator::getFilteredDistance(float distEst, float distTraveled) {
    //qDebug() << "getFilteredDistance";
    float ratio = 0.1;
    if (dataPoints < 8) {
        ratio = 1.0/(dataPoints+1.0);
        dataPoints++;
    }

    //qDebug() << "estimated with speed" << distanceLastTimestamp + distTraveled << distEst;
    if (distTraveled > 10000) {
        if (distEst - distanceLastTimestamp > 10000 || distanceLastTimestamp - distEst > 10000) {
            return distanceLastTimestamp;
        }
        else {
            return distEst;
        }
    }
    /*if (distanceLastTimestamp + distTraveled - distEst > 250 || distanceLastTimestamp +distTraveled - distEst < - 250) {
       return distanceLastTimestamp + distTraveled;
    }*/
    //onder de aanname dat alles fatsoenlijk werkt

    //qDebug() << "ratio " << ratio;
    //qDebug() << distEst;
    return ratio * distEst + (1.0 - ratio) * (distanceLastTimestamp + distTraveled);
}

void DistanceCalculator::updateVariables(quint64 timestamp, float dist, float lat, float lon, float speed) {
    lastLat = lat;
    lastLon = lon;
    distanceLastTimestamp = dist;
    lastTimestamp = timestamp;
    lastSpeed = speed;
}
