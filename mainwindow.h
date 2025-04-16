#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class GridManager;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private:
    void setupUI();

    GridManager* m_gridManager;
};

#endif // MAINWINDOW_H
