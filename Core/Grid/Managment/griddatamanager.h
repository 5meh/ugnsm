// griddatamanager.h
#ifndef GRIDDATAMANAGER_H
#define GRIDDATAMANAGER_H

#include <QObject>
#include <QVector>
#include <QHash>
#include <QNetworkInterface>
#include <QPoint>
#include <QMessageBox>
#include <QSharedPointer>

#include "../Utilities/Parser/iparser.h"

#include "../Network/Information/networkinfo.h"

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
    explicit GridDataManager(QObject* parent = nullptr);
    virtual ~GridDataManager();
    QSharedPointer<NetworkInfoModel> cellData(QPoint indx) const;

    int getRows() const;
    int getCols() const;
    int getCapacity() const;
    void initializeGrid(int rows, int cols);
    void swapCells(QPoint from, QPoint to);
    void setUpdatesPaused(bool paused);

public slots:
    void refreshData();
    void triggerRefresh();

signals:
    void gridDimensionsChanged();
    void cellChanged(QPoint indx, QSharedPointer<NetworkInfoModel>);
    void gridReset();

private slots:
    void handleParsingCompleted(const QVariant& result);
    void handleNetworkStats(QString mac, quint64 rxSpeed, quint64 txSpeed);

private:
    void handleNetworkStatsImpl(QString mac, quint64 rxSpeed, quint64 txSpeed);
    void handleParsingCompletedImpl(QVariant result);
    void swapCellsImpl(QPoint from, QPoint to);
    void clearGrid();
    void updateMacMap();
    void initializeGridWithData(const QList<NetworkInfoPtr>& allInfos);
    void showBestNetworkChangedMessage(NetworkInfoPtr newBest);
    void applyUpdates(const QVector<QPair<QPoint, NetworkInfoPtr>>& updates);
    void fullGridUpdate(const QList<NetworkInfoPtr>& allInfos);
    void incrementalUpdate(const QList<NetworkInfoPtr>& allInfos);
    void keepBestUpdate(const QList<NetworkInfoPtr>& allInfos);
    void updateGridWithData(const QList<NetworkInfoPtr>& allInfos);
    void updateTrackedMacs();
    //void updateRefreshTask();

    QAtomicInt m_refreshInProgress{0};
    NetworkMonitor* m_monitor;
    std::shared_ptr<IParser> m_parser;
    std::shared_ptr<INetworkSortStrategy> m_sorter;
    QVector<QVector<QSharedPointer<NetworkInfoModel>>> m_data;
    QHash<QString, QPoint> m_macIndex;
    QHash<QString, QString> m_interfaceToMac;

    size_t m_validDataCount;
    bool m_updatesPaused = false;
    QList<QList<NetworkInfoPtr>> m_queuedUpdates;
};

#endif // GRIDDATAMANAGER_H
