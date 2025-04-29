#ifndef GRIDDATAMANAGER_H
#define GRIDDATAMANAGER_H

#include <QObject>
#include <QVector>
#include <QHash>
#include <QNetworkInterface>
#include <QPoint>

#include "../Utilities/Parser/iparser.h"

class NetworkInfoModel;
class IParser;
class INetworkSortStrategy;
class NetworkMonitor;

class GridDataManager : public QObject
{
    Q_OBJECT
public:
    explicit GridDataManager(QObject* parent = nullptr);
    virtual ~GridDataManager();
    NetworkInfoModel* cellData(QPoint indx) const;

    int getRows() const;//TODO:mb remowe later
    int getCols() const;//TODO:mb remowe later
    void initializeGrid(int rows, int cols);
    void swapCells(QPoint from, QPoint to);

signals:
    void modelChanged();
    void gridDimensionsChanged();
    void parsingFailed(const QStringList& warnings);
    void networkHighlightChanged(int row, int col);

    void cellChanged(QPoint indx);
    void gridReset();

private slots:
    void handleParsingCompleted(const QVariant& result);
    void handleNetworkStats(const QString& mac,
                            quint64 rxSpeed,
                            quint64 txSpeed);
    void refreshData();

private:
    void clearGrid();
    void updateMacMap();//TODO: mb remove later

    NetworkMonitor* m_monitor;
    std::shared_ptr<IParser> m_parser;
    std::shared_ptr<INetworkSortStrategy> m_sorter;
    QVector<QVector<NetworkInfoModel*>> m_data;
    //QHash<QString, NetworkInfoModel*> m_macMap;//TODO:mb remove we got not so many network so it probably useless to use QHash
    QHash<QString, QPoint> m_macIndex;
};

#endif // GRIDDATAMANAGER_H
