#include "mainwindow.h"
#include "Core/Grid/Managment/gridmanager.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
    m_gridManager(new GridManager(this))
{
    setupUI();
}

void MainWindow::setupUI()
{
    setCentralWidget(m_gridManager->view());
    resize(1280, 720);
    setWindowTitle("Network Dashboard");
}
