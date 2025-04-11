#ifndef GRIDDATAMANAGER_H
#define GRIDDATAMANAGER_H

#include <QObject>

class NetworkInfoModel;

class GridDataManager : public QObject
{
    Q_OBJECT
public:
    explicit GridDataManager(int rows, int cols, QObject* parent = nullptr);

    NetworkInfoModel* cellData(int row, int col) const;

    bool setSize(int newRows, int newCols);

    int getRows() const;
    void setRows(int newRows);//TODO: remove or refactor to change grid size dynamicly

    int getCols() const;
    void setCols(int newCols);//TODO: remove or refactor to change grid size dynamicly

public slots:
    // Swap the data in two cells.
    void swapCellData(int row1, int col1, int row2, int col2);
    void initializeData(int rows, int cols);
    void refreshGrid();

signals:
    void modelChanged();//TODO: mb rework to update certain cell

    //void rowsChanged();
    //void colsChanged();

private:
    //Q_PROPERTY(int rows READ rows WRITE setRows NOTIFY rowsChanged FINAL)
    //Q_PROPERTY(int cols READ cols WRITE setCols NOTIFY colsChanged FINAL)

    QVector<QVector<NetworkInfoModel*>> m_data;

    //int m_rows;
    //int m_cols;
};

#endif // GRIDDATAMANAGER_H
