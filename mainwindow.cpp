#include "mainwindow.h"
#include "UI/Components/Grid/GridViewManager/gridviewmanager.h"
#include "Core/Grid/Managment/gridmanager.h"


#include <QResizeEvent>
#include <QStatusBar>
#include <QMessageBox>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    //setWindowIcon(QIcon(":/icons/app_icon"));

    m_gridManager.reset(new GridManager(this));
    setupUI();
    setupConnections();
    updateWindowTitle();
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUI()
{
    if (!m_gridManager || !m_gridManager->getView())
    {
        QMessageBox::critical(this, "Initialization Error",
                              "Failed to initialize grid system");
        qFatal("Grid manager initialization failed");
    }

    setCentralWidget(m_gridManager->getView());
    statusBar()->showMessage("Ready", 3000);

    // Initial window setup
    const QSize initialSize(1280, 720);
    resize(initialSize);
    setMinimumSize(800, 600);
}

void MainWindow::setupConnections()
{
    connect(m_gridManager.data(), &GridManager::gridDimensionsChanged,
            this, &MainWindow::handleGridDimensionsChanged);

    // connect(m_gridManager->getView(), &GridViewManager::cellClicked,
    //         this, &MainWindow::handleCellClicked);

    // connect(m_gridManager.data(), &GridManager::errorOccurred,
    //         this, &MainWindow::handleGridError);
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);
    if(m_gridManager && m_gridManager->getView())
    {
        m_gridManager->getView()->update();
    }
}

void MainWindow::handleGridDimensionsChanged()
{
    updateWindowTitle();
    statusBar()->showMessage(
        QString("Grid resized to %1x%2").arg(m_gridManager->getRows()).arg(m_gridManager->getCols()),
        2000
        );
}

void MainWindow::handleCellClicked(int row, int column)
{
    statusBar()->showMessage(
        QString("Cell clicked at [%1,%2]").arg(row).arg(column),
        1500
        );
}

void MainWindow::handleGridError(const QString& errorMessage)
{
    QMessageBox::warning(this, "Grid Error", errorMessage);
    statusBar()->showMessage(errorMessage, 5000);
}

void MainWindow::updateWindowTitle()
{
    const QString dimensions = m_gridManager ?
                                   QString(" [%1x%2]").arg(m_gridManager->getRows()).arg(m_gridManager->getCols()) : "";

    setWindowTitle(QString("Network Dashboard%1 - v%2")
                       .arg(dimensions)
                       .arg(QCoreApplication::applicationVersion()));
}
