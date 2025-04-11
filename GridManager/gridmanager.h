#ifndef GRIDMANAGER_H
#define GRIDMANAGER_H

#include <QObject>

class GridDataManager;
class GridViewManager;

class GridManager : public QObject
{
    Q_OBJECT
public:
    explicit GridManager(QObject *parent = nullptr);

    ~GridManager();

    void setGridDimensions(int rows, int cols);

    GridViewManager* view() const;
private:
    GridDataManager* m_dataManager;
    GridViewManager* m_viewManager;

    int m_rows;
    int m_cols;
};

#endif // GRIDMANAGER_H
