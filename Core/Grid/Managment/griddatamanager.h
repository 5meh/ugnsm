#ifndef GRIDDATAMANAGER_H
#define GRIDDATAMANAGER_H

#include <QObject>
#include <QVector>
#include <QHash>
#include <QNetworkInterface>

class NetworkInfoModel;
class IParser;
class INetworkSortStrategy;
class NetworkMonitor;

class GridDataManager : public QObject
{
    Q_OBJECT
public:
    explicit GridDataManager(IParser* parser, INetworkSortStrategy *sorter, QObject* parent = nullptr);
    virtual ~GridDataManager();
    NetworkInfoModel* cellData(int row, int col) const;

    int getRows() const;//TODO:mb remowe later
    int getCols() const;//TODO:mb remowe later
    void initializeGrid(int rows, int cols);
    void swapCells(int fromRow, int fromCol, int toRow, int toCol);

signals:
    void modelChanged();
    void gridDimensionsChanged();

private slots:
    void handleParsingCompleted(const QVariant& result);
    void handleNetworkStats(const QString& mac,
                            quint64 rxSpeed,
                            quint64 txSpeed);
    void refreshData();

private:
    void clearGrid();
    void updateMacMap();

    NetworkMonitor* m_monitor;
    INetworkSortStrategy* m_sorter;
    IParser* m_parser;
    QVector<QVector<NetworkInfoModel*>> m_data;
    QHash<QString, NetworkInfoModel*> m_macMap;
};

#endif // GRIDDATAMANAGER_H
