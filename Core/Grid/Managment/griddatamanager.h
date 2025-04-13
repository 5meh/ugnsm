#ifndef GRIDDATAMANAGER_H
#define GRIDDATAMANAGER_H

#include <QObject>
#include <QVector>
#include <QNetworkInterface>

class NetworkInfoModel;

class GridDataManager : public QObject
{
    Q_OBJECT
public:
    explicit GridDataManager(QObject* parent = nullptr);

    NetworkInfoModel* cellData(int row, int col) const;
    void initializeData(int rows, int cols);
    void refreshGrid();

signals:
    void modelChanged();

private:
    QList<QNetworkInterface> getSortedInterfaces() const;

    QVector<QVector<NetworkInfoModel*>> m_data;    
};

#endif // GRIDDATAMANAGER_H
