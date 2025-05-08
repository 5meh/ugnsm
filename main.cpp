#include "mainwindow.h"

#include "componentregistry.h"

#include "Core/Network/NetworkSortingStrategies/speedsortstrategy.h"
#include "Utilities/Logger/logger.h"
#include "Utilities/Parser/networkethernetparser.h"
#include "Core/Network/Information/networkinfo.h"

#include <QApplication>
#include <QFile>

void configureSystem()
{
    ComponentRegistry::registerComponent<INetworkSortStrategy, SpeedSortStrategy>();
    ComponentRegistry::registerComponent<IParser, NetworkEthernetParser>();

    Logger::instance().log(Logger::Info, "Components registered", "MainWindow");
}

int main(int argc, char *argv[])
{
    qRegisterMetaType<NetworkInfo*>("NetworkInfo*");
    qRegisterMetaType<QList<NetworkInfo*>>("QList<NetworkInfo*>");

    QApplication a(argc, argv);


    QFile styleFile(QStringLiteral("://styles.qss"));
    if (!styleFile.open(QFile::ReadOnly | QFile::Text))//TODO:remove later
    {
        qWarning() << "Failed to open style.qss:" << styleFile.errorString();
    }
    else
    {
        QByteArray styleData = styleFile.readAll();
        QString qss = QString::fromUtf8(styleData);
        //qDebug() << "QSS is\n" << qss.left(200) << "...";
        a.setStyleSheet(qss);
    }
    configureSystem();

    MainWindow w;
    w.show();
    return a.exec();
}
