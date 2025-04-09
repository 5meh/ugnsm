#include "gridmodelmanager.h"

GridModelManager::GridModelManager(int rows, int cols, QObject* parent)
    : QObject{parent}
{

}

NetworkInfoModel* GridModelManager::cellData(int row, int col) const
{
    if (row < 0 || row >= m_rows || col < 0 || col >= m_cols)
        return nullptr;

    return m_data[row][col];
}

bool GridModelManager::setSize(int newRows, int newCols)
{
    if (newRows < 0 || newCols < 0)
    {
        qWarning() << "setSize: Invalid dimensions (" << newRows << "," << newCols << ")";
        return false;
    }

    if(newRows != getRows())
    {
        if (newRows < oldRows)
        {
            for (int row = newRows; row < oldRows; ++row)
            {
                for (int col = 0; col < m_data[row].size(); ++col)
                {
                    if (m_data[row][col] != nullptr)
                    {
                        delete m_data[row][col];
                        m_data[row][col] = nullptr;
                    }
                }
            }
            m_data.resize(newRows);
        }
        else if (newRows > oldRows)
        {
            for (int row = oldRows; row < newRows; ++row)
            {
                QVector<NetworkInfoModel*> newRow;
                m_data.append(newRow);
            }
        }
    }

    if(newCols != getCols())
    {
        for (int row = 0; row < m_data.size(); ++row)
        {
            int oldCols = m_data[row].size();
            if (newCols < oldCols)
            {
                for (int col = newCols; col < oldCols; ++col)
                {
                    if (m_data[row][col] != nullptr)
                    {
                        delete m_data[row][col];
                        m_data[row][col] = nullptr;
                    }
                }
                m_data[row].resize(newCols);
            }
            else if (newCols > oldCols)
            {
                for (int col = oldCols; col < newCols; ++col)
                {
                    m_data[row].append(nullptr);
                }
            }
        }
    }

    qDebug() << "Grid resized to:" << newRows << "rows and" << newCols << "columns.";
    return true;
}

int GridModelManager::getRows() const
{
    return m_data.size();
}

void GridModelManager::setRows(int newRows)
{
    if (m_rows == newRows)
        return;
    setSize(newRows, getCols());//TODO:mb change return type to bool
    //m_rows = newRows;
    //emit rowsChanged();
}

int GridModelManager::getCols() const
{
    m_data.isEmpty() ? 0 : m_data.first().size();
}

void GridModelManager::setCols(int newCols)
{
    if (m_cols == newCols)
        return;
    setSize(getRows(), newCols);//TODO:mb change return type to bool
    // m_cols = newCols;
    // emit colsChanged();
}

void GridModelManager::swapCellData(int row1, int col1, int row2, int col2)
{
    if (row1 < 0 || row1 >= m_rows || row2 < 0 || row2 >= m_rows ||
        col1 < 0 || col1 >= m_cols || col2 < 0 || col2 >= m_cols)
        return;
    qSwap(m_data[row1][col1], m_data[row2][col2]);
    qDebug() << "LogicalManager: Swapped data between (" << row1 << "," << col1
             << ") and (" << row2 << "," << col2 << ")";
    emit modelChanged();
}
