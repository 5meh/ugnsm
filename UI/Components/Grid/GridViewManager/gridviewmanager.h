#ifndef GRIDVIEWMANAGER_H
#define GRIDVIEWMANAGER_H

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
    void setCell(int row, int col, GridCellWidget* widget);
    void updateCell(int row, int col, NetworkInfoModel* model);
    void clearCell(int row, int col);

    int gridRows() const { return m_cells.size(); }
    int gridCols() const { return m_cells.isEmpty() ? 0 : m_cells[0].size(); }

signals:
    void cellSwapRequestToDataManager(QPoint from, QPoint to);

// protected:
//     void dragEnterEvent(QDragEnterEvent* event) override;
//     void dragMoveEvent(QDragMoveEvent* event) override;
//     void dragLeaveEvent(QDragLeaveEvent* event) override;
//     void dropEvent(QDropEvent* event) override;

private slots:
    void handleSwapRequested(QPoint source, QPoint target);

private:
    void clearGrid();
    void highlightCell(int row, int col);
    void clearHighlight();
    QPoint getCellIndexFromPos(const QPoint& indx);
    void updateCellContent(int row, int col, NetworkInfoModel* model);
    GridCellWidget* createCellWidgetForModel(NetworkInfoModel* model);

    QGridLayout* m_gridLayout;
    QVector<QVector<GridCellWidget*>> m_cells;
    GridCellWidget* m_highlightedCell = nullptr;
};

#endif // GRIDVIEWMANAGER_H
