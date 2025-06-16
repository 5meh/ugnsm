#include "mainwindow.h"
#include "UI/Components/Grid/GridViewManager/gridviewmanager.h"
#include "Core/Grid/Managment/gridmanager.h"
#include "UI/Components/DialogWindows/settingsdialog.h"
#include "Core/globalmanager.h"
#include "Utilities/Logger/logger.h"

#include <QResizeEvent>
#include <QStatusBar>
#include <QMessageBox>
#include <QMenuBar>
#include <QMenu>
#include <QStyle>
#include <QToolBar>
#include <QScrollArea>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
    m_toolbar(new QToolBar(this)),
    m_settingsDialog(new SettingsDialog (this))
{
    m_gridManager.reset(new GridManager(this));
    setupUI();
    setupConnections();
}

MainWindow::~MainWindow()
{

}

void MainWindow::setupUI()
{
    QMenu* settingsMenu = menuBar()->addMenu(tr("Settings"));
    settingsMenu->addAction(tr("Configure..."), m_settingsDialog, &SettingsDialog::exec);

    m_toolbar = addToolBar(tr("Actions"));

    QIcon reloadIcon = style()->standardIcon(QStyle::SP_BrowserReload);
    m_manualRefreshAction = m_toolbar->addAction(reloadIcon, tr("Refresh Now"));

    bool autoRefresh = GlobalManager::settingsManager()->getAutoRefresh();
    m_toolbar->setVisible(!autoRefresh);
    m_manualRefreshAction->setVisible(!autoRefresh);

    if (!m_gridManager)
    {
        QMessageBox::critical(this, "Initialization Error",
                              "Failed to initialize grid system");
    }

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setWidget(m_gridManager->gridWidget());
    setCentralWidget(m_scrollArea);

    statusBar()->showMessage("Ready", 3000);

    const QSize initialSize(1280, 720);
    resize(initialSize);
    setMinimumSize(800, 600);
}

void MainWindow::setupConnections()
{
    connect(m_settingsDialog, &SettingsDialog::settingsChanged,
            this, &MainWindow::handleSettingsChanged);

    connect(GlobalManager::settingsManager(), &SettingsManager::autoRefreshChanged,
            this, &MainWindow::handleAutoRefreshChanged);

    connect(m_manualRefreshAction, &QAction::triggered,
            this, &MainWindow::onManualRefresh);
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);
    if(m_gridManager)
    {
        m_gridManager->gridWidget()->update();
    }
}

void MainWindow::handleGridError(const QString& errorMessage)
{
    QMessageBox::warning(this, "Grid Error", errorMessage);
    statusBar()->showMessage(errorMessage, 5000);
}

void MainWindow::handleSettingsChanged()
{
    m_gridManager->refresh();

    statusBar()->showMessage(tr("Manual refresh triggered"), 1500);
}

void MainWindow::onManualRefresh()
{
    if (m_gridManager)
    {
        GlobalManager::dragManager()->endDrag();
    }

    if (GlobalManager::settingsManager()->getAutoRefresh())
        GlobalManager::taskScheduler()->cancelRepeating("data_refresh");

    m_gridManager->refresh();
    statusBar()->showMessage(tr("Manual refresh triggered"), 1500);

    Logger::instance().log(Logger::Info, "Manual refresh triggered", "MainWindow");
}

void MainWindow::handleAutoRefreshChanged(bool enabled)
{
    m_toolbar->setVisible(!enabled);
    m_manualRefreshAction->setVisible(!enabled);

    m_gridManager->handleAutoRefreshChanged(enabled);
    Logger::instance().log(Logger::Info,
                           QString("Auto-refresh changed: %1").arg(enabled ? "Enabled" : "Disabled"),
                           "GridDataManager");
}
