#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QDebug>
#include <QtCore/QLocale>
#include <QtCore/QDataStream>

#include <mod_ukl.h>
#include <visualization.h>

CVisualization::CVisualization( QObject *parent ) : QObject{ parent }, minLevel( 10 ), maxLevel( -109 )
{
    // Соединяем обмен данными с классом чтения COM порта
    connect( &m_thread, &ReceiverThread::Request, this,&CVisualization::receivePortData );
}

void CVisualization::parseTheFile( const QString &nameFile )
{
    // Соединяем QML визуализацию
    if( nameFile == "" ) {
        emit connectData();
    }
    // Открываем файл
    QFile file( nameFile );
    if ( !file.open( QIODevice::ReadOnly ) ) {
        qDebug() << file.errorString();
        return;
    }

    // Парсим
    while ( !file.atEnd() ) {
        QByteArray line = file.readLine();

        QString iterationDataLine = line.split('\r').first();
        iterationDataLine.remove(' ');

        QStringList tempTokenList = iterationDataLine.split(',');

        if( tempTokenList.length() != 6 ) {
            return;
        }

        // Формируем одну итерацию данных
        if( tempTokenList[3].toInt() > 0 ) {
            formFileData( tempTokenList );
        }
    }
    file.close();
}

void CVisualization::readingComPort( const QString connectToPort )
{
    qDebug() << "Connecting to " << connectToPort;
    iterationInput.clear();                         // Чистим пул входных данных перед чтением


    int waitTimeout = 100;
    m_thread.startReceiver(connectToPort, waitTimeout);
}

void CVisualization::formFileData( const QStringList &dataLine )
{
    QVariant prevIteration;
    if(iterationInput.length() > 0)
        prevIteration = iterationInput[iterationInput.length()-1];

    // Переводим токены в конкретные типы
    CDataPoint tempStruct;

    tempStruct.time         = dataLine[0].toUInt();
    tempStruct.latitude     = dataLine[1].toDouble();
    tempStruct.longitude    = dataLine[2].toDouble();
    tempStruct.height       = dataLine[3].toUInt();
    tempStruct.centralFreq  = dataLine[4].toUInt();
    tempStruct.level        = dataLine[5].toInt();

    if(iterationInput.length() > 0){
        if((fabs(tempStruct.latitude - prevIteration.toMap()["latitude"].toDouble()) > 1)
                || (fabs(tempStruct.longitude - prevIteration.toMap()["longitude"].toDouble()) > 1)){
            tempStruct.latitude     = prevIteration.toMap()["latitude"].toDouble();
            tempStruct.longitude    = prevIteration.toMap()["longitude"].toDouble();
        }
    }

    tempStruct.spike = ( tempStruct.level != DEFAULT_LVL )? true : false;

    // Сохраняем данные
    iterationInput.push_back( tempStruct.ToQVariantMap() );

    // Обновляем абсолютные величины
    if( tempStruct.level > maxLevel ){
        maxLevel = tempStruct.level;
    } else if(( tempStruct.level < minLevel ) && ( tempStruct.level != DEFAULT_LVL )){
        minLevel = tempStruct.level;
    }
    setMinMaxValues();

    emit this->dataUpdated();
}

void CVisualization::formComPortData( const MacBIN dataLine )
{
    QVariant prevIteration;
    if(iterationInput.length() > 0)
        prevIteration = iterationInput[iterationInput.length()-1];

    for(int i = 0; i < 4; i++){
        CDataPoint tempStruct;
        unsigned int tempFreq;
        switch(i){
            case 0:
                tempFreq = 450;
                break;
            case 1:
                tempFreq = 750;
                break;
            case 2:
                tempFreq = 2400;
                break;
            case 3:
                tempFreq = 4200;
                break;
        }

        tempStruct.time         = dataLine.time;
        tempStruct.latitude     = dataLine.lat;
        tempStruct.longitude    = dataLine.lon;
        tempStruct.height       = dataLine.alt;
        tempStruct.centralFreq  = tempFreq;
        tempStruct.level        = (int)dataLine.bandlevel[i];

        tempStruct.spike = ( tempStruct.level != DEFAULT_LVL )? true : false;
        iterationInput.push_back( tempStruct.ToQVariantMap() );
        if( tempStruct.level > maxLevel ){
            maxLevel = tempStruct.level;
        } else if(( tempStruct.level < minLevel ) && ( tempStruct.level != DEFAULT_LVL )){
            minLevel = tempStruct.level;
        }

        setMinMaxValues();
        emit this->dataUpdated();
    }
}

void CVisualization::countComPorts(){
    portsNameInfo.clear();
    // Считаем имена активных COM портов
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    for( int i = 0; i < ports.count(); i++ ){
        portsNameInfo.push_back( ports.at( i ).portName() );
        qDebug() << "name: " << ports.at( i ).portName();
        qDebug() << "vendorID: " << ports[i].vendorIdentifier();
        qDebug() << "productID: " << ports[i].productIdentifier();
    }
}

void CVisualization::setMinMaxValues()
{
    if(iterationInput.length() == 1){       // Первый элемент
        maxLat = iterationInput[0].toMap()["latitude"].toDouble();
        minLat = iterationInput[0].toMap()["latitude"].toDouble();
        maxLon = iterationInput[0].toMap()["longitude"].toDouble();
        minLon = iterationInput[0].toMap()["longitude"].toDouble();
    }else{
        double lat = iterationInput[iterationInput.length()-1].toMap()["latitude"].toDouble();
        double lon = iterationInput[iterationInput.length()-1].toMap()["longitude"].toDouble();

        if( lon > maxLon ){
            maxLon = lon;
        } else if( lon < minLon ){
            minLon = lon;
        }

        if( lat > maxLat ){
            maxLat = lat;
        } else if( lat < minLat ){
            minLat = lat;
        }
    }
}


void CVisualization::selectFile( const QString &newValue )
{
    inputFileAddr = newValue;
    iterationInput.clear();
    parseTheFile( inputFileAddr );

    emit connectData();
}

void CVisualization::receivePortData( const QByteArray newData )
{
    MacBIN receivedData;
    memcpy(&receivedData, newData.data(), MSG_LEN);

    CMod_ukl::GetInstance()->SndData(newData);

    formComPortData(receivedData);
}

void CVisualization::receiveEthernetData( QByteArray newData )
{
    int dataLen = newData.length();
    if( dataLen >= MSG_LEN ) {
        const char *buf = newData.data();
        if( !strncmp( buf, MSG_HEADER, 3 ) ) {
            MacBIN datarcv;
            memcpy( &datarcv, buf, MSG_LEN );
            char datalen = datarcv.len;
            if( datalen == DATA_LEN ) {
                int scCalc = 0;
                for( int i=0; i<DATA_LEN; i++ )
                    scCalc += (unsigned char) buf[4+i];
                scCalc &= 0xFFFF;
                if( scCalc != datarcv.cs ) {
                    qDebug() << "Error CS: scCalc: " << scCalc << ", csRcvd: " << datarcv.cs;
                    return;
                }
                formComPortData( datarcv );
            }
        }
    }
}
