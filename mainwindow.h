#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QScopedPointer>

class GridManager;
class SettingsDialog;
QT_FORWARD_DECLARE_CLASS(QResizeEvent)
QT_FORWARD_DECLARE_CLASS(QScrollArea)

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

    GridManager* gridManager() const { return m_gridManager.data(); }


protected:
    void resizeEvent(QResizeEvent* event) override;

private slots:
    //void handleGridDimensionsChanged();
    void handleGridError(const QString& errorMessage);
    void handleSettingsChanged();
    void onManualRefresh();
    void handleAutoRefreshChanged(bool enabled);

private:
    void setupUI();
    void setupConnections();
    //void updateWindowTitle();

    QScrollArea* m_scrollArea;
    QToolBar* m_toolbar;
    QAction*  m_manualRefreshAction;
    QScopedPointer<GridManager> m_gridManager;
    SettingsDialog* m_settingsDialog;
};

#endif // MAINWINDOW_H
