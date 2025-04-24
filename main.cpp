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
    if(styleFile.open(QFile::ReadOnly))
    {
        a.setStyleSheet(styleFile.readAll());
    }

    configureSystem();

    MainWindow w;
    w.show();
    return a.exec();
}
