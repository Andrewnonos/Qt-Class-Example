#ifndef DATAPOINT_H
#define DATAPOINT_H

#include <QtCore/QObject>

class CDataPoint {                          ///< Класс для формирования данных
public:
    unsigned int time;                      ///< Время
    double latitude;                        ///< Широта
    double longitude;                       ///< Долгота
    unsigned int height;                    ///< Высота
    unsigned int centralFreq;               ///< Центральная частота
    int level;                              ///< Уровень сигнала
    bool spike;                             ///< Наличие скачка

    QVariantMap ToQVariantMap() {           ///< Преобразование в QVariantMap для QML
        QVariantMap result;
        result["time"]          = time;
        result["latitude"]      = latitude;
        result["longitude"]     = longitude;
        result["height"]        = height;
        result["centralFreq"]   = centralFreq;
        result["level"]         = level;
        result["spike"]         = spike;

        return result;
    }
};


#endif // DATAPOINT_H