#ifndef GRIDVIEWMANAGER_H
#define GRIDVIEWMANAGER_H

#include <QWidget>
#include <QGridLayout>
#include <QVector>

class GridCellWidget;

class GridViewManager : public QWidget
{
    Q_OBJECT
public:
    explicit GridViewManager(QWidget *parent = nullptr);
    ~GridViewManager();

    void setGridSize(int rows, int cols);
    GridCellWidget* cellAt(int row, int col) const;
    void setCell(int row, int col, GridCellWidget* widget);

signals:
    void cellSwapRequested(int fromRow, int fromCol, int toRow, int toCol);

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private slots:
    void handleSwapRequested(GridCellWidget* source, GridCellWidget* target);

private:
    void clearGrid();
    void highlightCell(int row, int col);
    void clearHighlight();
    QPoint parseCellPosition(const QString& cellId) const;

    QGridLayout* m_gridLayout;
    QVector<QVector<GridCellWidget*>> m_cells;
    GridCellWidget* m_highlightedCell = nullptr;
};

#endif // GRIDVIEWMANAGER_H
