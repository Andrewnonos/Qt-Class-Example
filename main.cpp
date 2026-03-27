#include <QJsonObject>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include <visualization.h>
#include <mod_ukl.h>

int main(int argc, char *argv[])
{
    CLoraModem lora;
    lora.connect();
    QGuiApplication app(argc
    , argv);

    QQmlApplicationEngine engine;
    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreated,
        &app,
        [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);

    QQmlContext* myContext = engine.rootContext();

    CMod_ukl::GetInstance()->setWorkFrequency( "26647" );
    CMod_ukl::GetInstance()->SetRcvFrequency( "26647" );
    CMod_ukl::GetInstance()->Init();
    CMod_ukl::GetInstance()->SetMode(3);

    QByteArray newData = {""};
    CMod_ukl::GetInstance()->SndData( newData );

    CVisualization* module_ptr = new CVisualization();
    QObject::connect(CMod_ukl::GetInstance(), &CMod_ukl::Received, module_ptr, &CVisualization::receiveEthernetData );

    module_ptr->parseTheFile( "" );
    myContext->setContextProperty( "_visualization", module_ptr );

    engine.load(url);

    return app.exec();
}
