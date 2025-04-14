#ifndef GRIDDATAMANAGER_H
#define GRIDDATAMANAGER_H

#include <QObject>
#include <QVector>
#include <QNetworkInterface>

class NetworkInfoModel;
class IParser;

class GridDataManager : public QObject
{
    Q_OBJECT
public:
    explicit GridDataManager(IParser* parser, QObject* parent = nullptr);

    NetworkInfoModel* cellData(int row, int col) const;
    void initializeData(int rows, int cols);
    void refreshGrid();

signals:
    void modelChanged();
private slots:
    void handleParsingCompleted(const QVariant& result);

private:
    QList<QNetworkInterface> getSortedInterfaces() const;

    IParser* m_parser;
    QVector<QVector<NetworkInfoModel*>> m_data;    
};

#endif // GRIDDATAMANAGER_H
