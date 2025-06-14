#ifndef GRIDVIEWMANAGER_H
#define GRIDVIEWMANAGER_H

#include <functional>

#include <QWidget>
#include <QGridLayout>
#include <QVector>

class GridCellWidget;
class NetworkInfoModel;

class GridViewManager : public QWidget
{
    Q_OBJECT
public:
    explicit GridViewManager(QWidget *parent = nullptr);
    ~GridViewManager();

    void setGridSize(int rows, int cols);
    GridCellWidget* cellAt(int row, int col) const;
    void setCell(QPoint indx, GridCellWidget* widget);

    void clearCell(int row, int col);

    int gridRows() const { return m_cells.size(); }
    int gridCols() const { return m_cells.isEmpty() ? 0 : m_cells[0].size(); }

    void setUpdatesEnabled(bool enable);

public slots:
    void updateCell(QPoint indx, QSharedPointer<NetworkInfoModel> model);

signals:
    void cellSwapRequestToDataManager(QPoint from, QPoint to);
    void pauseGridUpdates(bool paused);

private slots:
    void handleSwapRequested(QPoint source, QPoint target);

private:
    void handleSwapRequestedImpl(QPoint source, QPoint target);
    void clearGrid();
    void highlightCell(int row, int col);
    void clearHighlight(int row, int col);
    QPoint getCellIndexFromPos(const QPoint& indx);
    GridCellWidget* createCellWidgetForModel(QSharedPointer<NetworkInfoModel> model);
    bool isPlaceholder(GridCellWidget* widget) const;
    void performSwap(QPoint source, QPoint target);

    bool m_updatesEnabled = true;
    std::function<void(QWidget*)> updateUI;//TODO:mb remove no need in it
    QGridLayout* m_gridLayout;
    QVector<QVector<GridCellWidget*>> m_cells;
    GridCellWidget* m_highlightedCell = nullptr;
};

#endif // GRIDVIEWMANAGER_H
