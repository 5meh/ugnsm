#include "mainwindow.h"

#include "componentsystem.h"

#include "Core/Network/NetworkSortingStrategies/speedsortstrategy.h"
#include "Utilities/Parser/networkethernetparser.h"

#include <QApplication>

void configureSystem(ComponentSystem& system)
{
    system.configure<SpeedSortStrategy>();
    system.configure<NetworkEthernetParser>(
        system.create<SpeedSortStrategy>()
        );
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    ComponentSystem system;
    configureSystem(system);

    MainWindow w(system);
    w.show();
    return a.exec();
}
