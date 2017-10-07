#ifndef COMBINEDMESSAGEGENERATOR_H
#define COMBINEDMESSAGEGENERATOR_H

#include "distancecalculator.h"
#include <QtMath>

class CombinedMessageGenerator
{
public:
    CombinedMessageGenerator();
    QByteArray getDistance(quint64 timestamp, QByteArray lat, QByteArray lon, QByteArray speed);
    DistanceCalculator distanceCalculator;

    QByteArray addFloats(QByteArray one, QByteArray two);
    QByteArray addFloats(QByteArray one, QByteArray two, QByteArray three);
    QByteArray multiplyFloats(QByteArray one, QByteArray two);
    QByteArray multiplyFloatQint32dv1000(QByteArray floatie, QByteArray quint32ie);
    QByteArray maxFloats(QByteArray one, QByteArray two);
    QByteArray subtractQint32dvFloats(QByteArray plus, float divisor, float divisor2, QByteArray minusOne, QByteArray minusTwo);
    QByteArray divideFloats(QByteArray first, QByteArray second);
    QByteArray maxQint32s(QList<QByteArray> quint32s);
    QByteArray getDistMK5();
    QByteArray getDistSpeed();

private:
    template <typename T>
    union TypeUnion {
        char bytes[sizeof(T)];
        T value;
    };

    template <typename T>
    T convertTo(QByteArray data) {
        TypeUnion<float> converter;
        for (int i = 0; i < sizeof(T); i++) {
            converter.bytes[i] = data.data()[i];
        }
        return converter.value;
    }

    template <typename T>
    QByteArray convertFrom(T value) {
        TypeUnion<float> converter;
        converter.value = value;
        QByteArray data(converter.bytes, sizeof(T));
        return data;
    }
};

#endif // COMBINEDMESSAGEGENERATOR_H
