#ifndef NETWORKDASHBOARD_H
#define NETWORKDASHBOARD_H

#include <QObject>
#include <QVector>
#include "Core/Network/Information/networkinfomodel.h"

class NetworkDashboard : public QObject
{
    Q_OBJECT

public:
    explicit NetworkDashboard(QObject* parent = nullptr);

    void addNetwork(NetworkInfoModel* viewModel);
    void moveNetwork(const QString& mac, int row, int col);
    QVector<QVector<NetworkInfoModel*>> currentLayout() const;
    NetworkInfoModel* viewModelAt(int row, int col) const;

signals:
    void layoutChanged();

public slots:
    void swapPositions(const QString& mac1, const QString& mac2);

private:
    void initializeGrid();
    QPair<int, int> findViewModel(NetworkInfoModel* viewModel) const;

    static constexpr int GRID_SIZE = 3;
    QVector<QVector<NetworkInfoModel*>> m_grid;
};

#endif // NETWORKDASHBOARD_H
