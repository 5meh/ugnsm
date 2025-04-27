#include "mainwindow.h"

#include "componentregistry.h"

#include "Core/Network/NetworkSortingStrategies/speedsortstrategy.h"
#include "Utilities/Parser/networkethernetparser.h"

#include <QApplication>
#include <QFile>

void configureSystem()
{
    ComponentRegistry::registerComponent<INetworkSortStrategy, SpeedSortStrategy>();
    ComponentRegistry::registerComponent<IParser, NetworkEthernetParser>();
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QFile styleFile(QStringLiteral("://styles.qss"));
    if (!styleFile.open(QFile::ReadOnly | QFile::Text))
    {
        qWarning() << "Failed to open style.qss:" << styleFile.errorString();
    }
    else
    {
        QByteArray styleData = styleFile.readAll();
        QString qss = QString::fromUtf8(styleData);
        qDebug() << "QSS is\n" << qss.left(200) << "...";
        a.setStyleSheet(qss);  // Or styleData
    }
    configureSystem();

    MainWindow w;
    w.show();
    return a.exec();
}
