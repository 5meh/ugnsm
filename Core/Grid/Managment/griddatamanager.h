#ifndef GRIDDATAMANAGER_H
#define GRIDDATAMANAGER_H

#include <QObject>
#include <QVector>
#include <QHash>
#include <QNetworkInterface>
#include <QPoint>
#include <QMutex>
#include <QWaitCondition>
#include <QAtomicInt>

#include "../Utilities/Parser/iparser.h"

class NetworkInfoModel;
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
    void swapCells(const QPoint& from, const QPoint& to);

signals:
    //void modelChanged();
    void gridDimensionsChanged();
    void parsingFailed(const QStringList& warnings);
    void networkHighlightChanged(int row, int col);

    void cellChanged(QPoint indx);
    void gridReset();

private slots:
    void handleParsingCompleted(const QVariant& result);
    void handleNetworkStats(const QString& mac, const quint64& rxSpeed, const quint64& txSpeed);
    void refreshData();

    void swapCellsImpl(const QPoint& from, const QPoint& to);
    void handleParsingCompletedImpl(QVariant result);
    void handleNetworkStatsImpl(const QString& mac, const quint64& rxSpeed, const quint64& txSpeed);

private:
    void processDataAsync();
    void safeSwapCells(QPoint from, QPoint to);
    void clearGrid();
    void updateMacMap();//TODO: mb remove later

    TaskScheduler* m_scheduler;
    QAtomicInt m_refreshInProgress{0};
    NetworkMonitor* m_monitor;
    std::shared_ptr<IParser> m_parser;
    std::shared_ptr<INetworkSortStrategy> m_sorter;
    QVector<QVector<NetworkInfoModel*>> m_data;
    QHash<QString, QPoint> m_macIndex;
};

#endif // GRIDDATAMANAGER_H
