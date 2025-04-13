#ifndef GRIDMANAGER_H
#define GRIDMANAGER_H

#include <QObject>

class GridDataManager;
class GridViewManager;

class GridManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int rows READ rows NOTIFY gridDimensionsChanged)
    Q_PROPERTY(int cols READ cols NOTIFY gridDimensionsChanged)
public:
    explicit GridManager(QObject* parent = nullptr);
    ~GridManager();

    int rows() const { return m_rows; }
    int cols() const { return m_cols; }
    void setGridDimensions(int rows, int cols);

    GridViewManager* view() const;

signals:
    void gridDimensionsChanged();

private:
    void syncManagers();

    GridDataManager* m_dataManager;
    QScopedPointer<GridViewManager> m_viewManager;

    int m_rows;
    int m_cols;
    static constexpr int GRID_ROWS_DEFAULT = 3;
    static constexpr int GRID_COLUMNS_DEFAUT = 3;
};

#endif // GRIDMANAGER_H
