#ifndef GRIDMANAGER_H
#define GRIDMANAGER_H

#include <QObject>

class GridDataManager;
class GridViewManager;
class IParser;
class INetworkSortStrategy;
class ParserType;
class SortStrategyType;

class ComponentSystem;

class GridManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int rows READ getRows NOTIFY gridDimensionsChanged)
    Q_PROPERTY(int cols READ getCols NOTIFY gridDimensionsChanged)
public:
    GridManager(QObject* parent = nullptr);
    virtual ~GridManager();

    int getRows() const;
    int getCols() const;
    void setGridDimensions(int rows, int cols);

    GridViewManager* getView() const;

signals:
    void gridDimensionsChanged();

private slots:
    void handleModelChanged();
    void handleSwapRequest(int fr, int fc, int tr, int tc);

private:
    void initializeView();
    void initializeData();
    void setupGridManager();
    void setupConnections();

    GridDataManager* m_dataManager;
    QScopedPointer<GridViewManager> m_viewManager;

    int m_rows;
    int m_cols;
    static constexpr int GRID_ROWS_DEFAULT = 3;
    static constexpr int GRID_COLUMNS_DEFAUT = 3;
};

#endif // GRIDMANAGER_H
