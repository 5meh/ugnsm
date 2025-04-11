#include "griddatamanager.h"

#include "networkinfo.h"
#include "networkinfomodel.h"

#include <QNetworkInterface>
#include <QTimer>

GridDataManager::GridDataManager(int rows, int cols, QObject* parent)
    : QObject{parent}
{
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &GridDataManager::refreshGrid);
    timer->start(5000);
}

NetworkInfoModel* GridDataManager::cellData(int row, int col) const
{
    if (row < 0 || row >= m_rows || col < 0 || col >= m_cols)
        return nullptr;

    return m_data[row][col];
}

bool GridDataManager::setSize(int newRows, int newCols)
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

int GridDataManager::getRows() const
{
    return m_data.size();
}

void GridDataManager::setRows(int newRows)
{
    if (m_rows == newRows)
        return;
    setSize(newRows, getCols());//TODO:mb change return type to bool
    //m_rows = newRows;
    //emit rowsChanged();
}

int GridDataManager::getCols() const
{
    m_data.isEmpty() ? 0 : m_data.first().size();
}

void GridDataManager::setCols(int newCols)
{
    if (m_cols == newCols)
        return;
    setSize(getRows(), newCols);//TODO:mb change return type to bool
    // m_cols = newCols;
    // emit colsChanged();
}

void GridDataManager::swapCellData(int row1, int col1, int row2, int col2)
{
    if (row1 < 0 || row1 >= m_rows || row2 < 0 || row2 >= m_rows ||
        col1 < 0 || col1 >= m_cols || col2 < 0 || col2 >= m_cols)
        return;
    qSwap(m_data[row1][col1], m_data[row2][col2]);
    qDebug() << "LogicalManager: Swapped data between (" << row1 << "," << col1
             << ") and (" << row2 << "," << col2 << ")";
    emit modelChanged();
}

void GridDataManager::initializeData(int rows, int cols)
{
    for (int i = 0; i < m_data.size(); ++i)
    {
        qDeleteAll(m_data[i]);
    }
    m_data.clear();

    QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
    QList<QNetworkInterface> ethernetInterfaces;
    for (const QNetworkInterface& interface : interfaces)
    {
        if (interface.type() == QNetworkInterface::Ethernet &&
            !interface.flags().testFlag(QNetworkInterface::IsLoopBack))
        {
            ethernetInterfaces.append(interface);
        }
    }
    // For this example, we ignore the possibility that rows*cols might exceed the number of interfaces.
    // We simply assign interfaces in order. If there are fewer interfaces than grid cells,
    // remaining cells may be filled with dummy models.
    m_data.resize(rows);
    int interfaceIndex = 0;
    for (int i = 0; i < rows; i++)
    {
        m_data[i].resize(cols);
        for (int j = 0; j < cols; j++)
        {
            NetworkInfoModel* modelPtr = nullptr;
            if (interfaceIndex < ethernetInterfaces.size())
            {
                // Create a NetworkInfo* from the interface.
                // Assume that NetworkInfo has a constructor: NetworkInfo(const QNetworkInterface&, QObject *parent);
                NetworkInfo* netInfo = new NetworkInfo(ethernetInterfaces.at(interfaceIndex), this);
                modelPtr = new NetworkInfoModel(netInfo, this);
                ++interfaceIndex;
            }
            else
            {
                // Create a dummy model if there is no interface available.
                NetworkInfo* netInfo = new NetworkInfo(/* dummy parameters */ , this);
                modelPtr = new NetworkInfoModel(netInfo, this);
            }
            m_data[i][j] = modelPtr;
        }
    }
    qDebug() << "GridDataManager: Data initialized with" << rows << "rows and" << cols << "cols.";
    emit modelChanged();
}

void GridDataManager::refreshGrid()
{
    emit modelChanged();
}
