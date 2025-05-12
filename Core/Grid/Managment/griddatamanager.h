// griddatamanager.h
#ifndef GRIDDATAMANAGER_H
#define GRIDDATAMANAGER_H

#include <QObject>
#include <QVector>
#include <QHash>
#include <QNetworkInterface>
#include <QPoint>

#include "../Utilities/Parser/iparser.h"

class NetworkInfoModel;
class NetworkInfo;
class IParser;
class INetworkSortStrategy;
class NetworkMonitor;
class TaskScheduler;

class GridDataManager : public QObject
{
    Q_OBJECT
public:
    explicit GridDataManager(TaskScheduler* scheduler, QObject* parent = nullptr);
    virtual ~GridDataManager();
    NetworkInfoModel* cellData(QPoint indx) const;

    int getRows() const;
    int getCols() const;
    void initializeGrid(int rows, int cols);
    void swapCells(QPoint from, QPoint to);

signals:
    void gridDimensionsChanged();
    void parsingFailed(const QStringList& warnings);
    void networkHighlightChanged(int row, int col);
    void cellChanged(QPoint indx, NetworkInfoModel*);
    void gridReset();

private slots:
    void handleParsingCompleted(const QVariant& result);
    void handleNetworkStats(QString mac, quint64 rxSpeed, quint64 txSpeed);
    void refreshData();

private:
    void handleNetworkStatsImpl(QString mac, quint64 rxSpeed, quint64 txSpeed);
    void handleParsingCompletedImpl(QVariant result);
    void swapCellsImpl(QPoint from, QPoint to);
    void clearGrid();
    void updateMacMap();
    void initializeGridWithData(const QList<NetworkInfo*>& allInfos);
    void updateGridWithData(const QList<NetworkInfo*>& allInfos);
    void showBestNetworkWarning();

    TaskScheduler* m_scheduler;
    QAtomicInt m_refreshInProgress{0};
    NetworkMonitor* m_monitor;
    std::shared_ptr<IParser> m_parser;
    std::shared_ptr<INetworkSortStrategy> m_sorter;
    QVector<QVector<NetworkInfoModel*>> m_data;
    QHash<QString, QPoint> m_macIndex;
    bool m_showBestNetworkWarning = true;
};

#endif // GRIDDATAMANAGER_H
