#ifndef NETWORKDASHBOARD_H
#define NETWORKDASHBOARD_H

#include <QObject>
#include <QVector>
#include "networkinfoviewmodel.h"

class NetworkDashboard : public QObject
{
    Q_OBJECT
public:
    explicit NetworkDashboard(QObject* parent = nullptr);

    void addInterface(NetworkInfoViewModel* viewModel);
    void moveInterface(const QString& mac, int row, int col);
    QVector<QVector<NetworkInfoViewModel*>> currentLayout() const;
    NetworkInfoViewModel* viewModelAt(int row, int col) const;

signals:
    void layoutChanged();

public slots:
    void swapPositions(const QString& mac1, const QString& mac2);

private:
    void initializeGrid();
    QPair<int, int> findViewModel(NetworkInfoViewModel* viewModel) const;

    static constexpr int GRID_SIZE = 3;
    QVector<QVector<NetworkInfoViewModel*>> m_grid;
};

#endif // NETWORKDASHBOARD_H
