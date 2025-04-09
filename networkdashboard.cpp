#include "networkdashboard.h"

NetworkDashboard::NetworkDashboard(QObject* parent)
    : QObject(parent)
{
    initializeGrid();
}

void NetworkDashboard::initializeGrid()
{
    m_grid.resize(GRID_SIZE);
    for(auto& row : m_grid) {
        row.resize(GRID_SIZE);
        row.fill(nullptr);
    }
}

QPair<int, int> NetworkDashboard::findViewModel(NetworkInfoModel *viewModel) const
{
    for(int row = 0; row < GRID_SIZE; ++row)
    {
        for(int col = 0; col < GRID_SIZE; ++col)
        {

            if(m_grid[row][col]->getMac() ==  viewModel->getMac())
            {
                return QPair<int, int>(row, col);
            }
        }
    }
}

void NetworkDashboard::addNetwork(NetworkInfoModel* viewModel)
{
    for(int row = 0; row < GRID_SIZE; ++row)
    {
        for(int col = 0; col < GRID_SIZE; ++col)
        {
            if(!m_grid[row][col])
            {
                m_grid[row][col] = viewModel;
                emit layoutChanged();
                return;
            }
        }
    }
}

void NetworkDashboard::moveNetwork(const QString& mac, int newRow, int newCol)
{
    if(newRow < 0 || newRow >= GRID_SIZE || newCol < 0 || newCol >= GRID_SIZE)
        return;

    for(int row = 0; row < GRID_SIZE; ++row)
    {
        for(int col = 0; col < GRID_SIZE; ++col)
        {
            if(m_grid[row][col] && m_grid[row][col]->getMac() == mac)
            {
                NetworkInfoModel* temp = m_grid[newRow][newCol];
                m_grid[newRow][newCol] = m_grid[row][col];
                m_grid[row][col] = temp;
                emit layoutChanged();
                return;
            }
        }
    }
}

QVector<QVector<NetworkInfoModel*>> NetworkDashboard::currentLayout() const
{
    return m_grid;
}

NetworkInfoModel* NetworkDashboard::viewModelAt(int row, int col) const
{
    if(row >= 0 && row < GRID_SIZE && col >= 0 && col < GRID_SIZE)
        return m_grid[row][col];
    return nullptr;
}

void NetworkDashboard::swapPositions(const QString& mac1, const QString& mac2)
{
    QPair<int, int> pos1 = {-1, -1};
    QPair<int, int> pos2 = {-1, -1};

    for(int row = 0; row < GRID_SIZE; ++row)
    {
        for(int col = 0; col < GRID_SIZE; ++col)
        {
            if(m_grid[row][col])
            {
                if(m_grid[row][col]->getMac() == mac1) pos1 = {row, col};
                if(m_grid[row][col]->getMac() == mac2) pos2 = {row, col};
            }
        }
    }

    if(pos1.first != -1 && pos2.first != -1)
    {
        qSwap(m_grid[pos1.first][pos1.second], m_grid[pos2.first][pos2.second]);
        emit layoutChanged();
    }
}
