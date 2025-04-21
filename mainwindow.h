#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QScopedPointer>

class GridManager;
class QResizeEvent;
//class ComponentSystem;

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
    void handleGridDimensionsChanged();
    void handleCellClicked(int row, int column);
    void handleGridError(const QString& errorMessage);

private:
    void setupUI();
    void setupConnections();
    void updateWindowTitle();

    QScopedPointer<GridManager> m_gridManager;
};

#endif // MAINWINDOW_H
