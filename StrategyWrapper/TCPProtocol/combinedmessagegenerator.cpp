#include "combinedmessagegenerator.h"

CombinedMessageGenerator::CombinedMessageGenerator()
{

}

QByteArray CombinedMessageGenerator::getDistance(quint64 timestamp, QByteArray lat, QByteArray lon, QByteArray speed) {
    return convertFrom<float>(distanceCalculator.getDistance(timestamp, convertTo<float>(lat), convertTo<float>(lon), convertTo<float>(speed)));
}

QByteArray CombinedMessageGenerator::addFloats(QByteArray one, QByteArray two) {
    return convertFrom<float>(convertTo<float>(one) + convertTo<float>(two));
}

QByteArray CombinedMessageGenerator::addFloats(QByteArray one, QByteArray two, QByteArray three) {
    return convertFrom<float>(convertTo<float>(one) + convertTo<float>(two) + convertTo<float>(three));
}

QByteArray CombinedMessageGenerator::multiplyFloats(QByteArray one, QByteArray two) {
    return convertFrom<float>(convertTo<float>(one) * convertTo<float>(two));
}

QByteArray CombinedMessageGenerator::multiplyFloatQint32dv1000(QByteArray floatie, QByteArray quint32ie) {
    return convertFrom<float>(0.001*(convertTo<float>(floatie) * convertTo<quint32>(quint32ie)));
}

QByteArray CombinedMessageGenerator::maxFloats(QByteArray one, QByteArray two) {
    if (convertTo<float>(one) > convertTo<float>(two)) {
        return one;
    }
    return two;
}

QByteArray CombinedMessageGenerator::maxQint32s(QList<QByteArray> quint8s) {
    qint32 index = 0;
    qint8 max = convertTo<quint8>(quint8s.at(0));
    for (int i = 1; i < quint8s.length(); i++) {
        quint8 thisInt = convertTo<quint8>(quint8s.at(i));
        if (thisInt > max) {
            max = thisInt;
            index = i;
        }
    }
    return convertFrom<float>((float) (max));
}

QByteArray CombinedMessageGenerator::subtractQint32dvFloats(QByteArray plus, float divisor, float divisor2, QByteArray plusTwo, QByteArray minusTwo) {
    //qDebug() << "P_batt" << ((convertTo<quint32>(plus))/divisor);
    //qDebug() << "P_solar" << (convertTo<float>(plusTwo)/divisor2);
    //qDebug() << "P_aux" << convertTo<float>(minusTwo);
    //qDebug() << "P_mot_bus" << ((float) ((convertTo<qint32>(plus)))/divisor + convertTo<float>(plusTwo)/divisor2 - convertTo<float>(minusTwo));
    return convertFrom<float>((float) ((convertTo<qint32>(plus))/divisor + convertTo<float>(plusTwo)/divisor2 - convertTo<float>(minusTwo)));
}

QByteArray CombinedMessageGenerator::divideFloats(QByteArray one, QByteArray two) {
    return convertFrom<float>(convertTo<float>(one) / convertTo<float>(two));
}

QByteArray CombinedMessageGenerator::getDistMK5() {
    return convertFrom<float>(distanceCalculator.distMK5);
}
QByteArray CombinedMessageGenerator::getDistSpeed() {
    return convertFrom<float>(distanceCalculator.distSpeed);
}


