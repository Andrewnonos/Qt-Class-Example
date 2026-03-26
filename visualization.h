#ifndef VISUALIZATION_H
#define VISUALIZATION_H

#include <QtCore/QObject>
#include <QtCore/QVariant>
#include <QtSerialPort/QSerialPortInfo>

#include <loramodem.h>

#include "receiverthread.hpp"
#include "mac_bin.h"

#define DEFAULT_LVL -110

///////////////////////////////////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////////////////////////////////

class CVisualization: public QObject            ///< Модуль визуализации
{
    Q_OBJECT
    Q_PROPERTY( QList<QVariant> iterationInput MEMBER iterationInput NOTIFY iterationInputChanged )
    Q_PROPERTY( QList<QVariant> portsNameInfo MEMBER portsNameInfo NOTIFY portsNameInfoChanged )

    Q_PROPERTY( double maxLat MEMBER maxLat NOTIFY maxLatChanged )
    Q_PROPERTY( double minLat MEMBER minLat NOTIFY minLatChanged )
    Q_PROPERTY( double maxLon MEMBER maxLon NOTIFY maxLonChanged )
    Q_PROPERTY( double minLon MEMBER minLon NOTIFY minLonChanged )

    Q_PROPERTY( double maxLevel MEMBER maxLevel NOTIFY maxLevelChanged )
    Q_PROPERTY( double minLevel MEMBER minLevel NOTIFY minLevelChanged )

public:
    explicit CVisualization( QObject *parent = nullptr );

    void parseTheFile( const QString & );                       ///< Парсим файл для получения данных дрона и дальнейшей обработки
    void formFileData( const QStringList & );                   ///< Собираем данные из файла
    void formComPortData( const MacBIN );                       ///< Собираем данные из COM порта

    void setMinMaxValues();                                     ///< Задаём средние значения

    Q_INVOKABLE void selectFile( const QString & );             ///< Выбор файла для визуализации записанных данных
    Q_INVOKABLE void readingComPort( const QString );           ///< Запуск чтения данных из COM порта
    Q_INVOKABLE void countComPorts();                           ///< Подсчёт работающих COM портов

private:
    QVariantList iterationInput;            ///< Обмен данными с QML
    QVariantList portsNameInfo;             ///< Информация о портах

    double maxLat;                          ///< Макс-значение широты
    double minLat;                          ///< Мин-значение широты
    double maxLon;                          ///< Макс-значение долготы
    double minLon;                          ///< Мин-значение долготы.

    CLoraModem lora;

    int minLevel;                           ///< Мин-уровень для диаметра кругов на карте
    int maxLevel;                           ///< Макс-уровень для диаметра кругов на карте

    ReceiverThread m_thread;                ///< Поток для получения данных с COM порта
    QString inputFileAddr = "";             ///< Строка адреса для подгрузки файла

    QString ipaddress = "192.168.100.89";                       ///< IP адрес УКЛ
    quint16 portSnd = 25001;                                    ///< Номер порта для передачи пакета
    quint16 portRcv = 25001;                                    ///< Номер порта для приема пакета

signals:
    void connectData();                         ///< Сигнал окончания считывания данных

    void iterationInputChanged();               ///< Фиксация изменений в отображённых данных

    void maxLatChanged();                       ///< Фиксация изменения макс-широты для QML
    void minLatChanged();                       ///< Фиксация изменения мин-широты для QML
    void maxLonChanged();                       ///< Фиксация изменения макс-долготы для QML
    void minLonChanged();                       ///< Фиксация изменения мин-долготы для QML

    void maxLevelChanged();                     ///< Фиксация изменения макс-уровня для QML
    void minLevelChanged();                     ///< Фиксация изменения мин-уровня для QML

    void portsNameInfoChanged();                ///< Уведомление QML об обновлении COM портов
    void dataUpdated();                         ///< Уведомление QML об обработке данных с COM порта

public slots:
    void receivePortData( const QByteArray );     ///<
    void receiveEthernetData( QByteArray );
};
Q_DECLARE_METATYPE(CDataPoint);

#endif // VISUALIZATION_H
