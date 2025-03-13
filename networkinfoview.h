#ifndef NETWORKINFOVIEW_H
#define NETWORKINFOVIEW_H

#include <QObject>
#include <QWidget>
//#include <QTableWidget>
QT_FORWARD_DECLARE_CLASS(QTableView)
QT_FORWARD_DECLARE_CLASS(QStandardItemModel);

class NetworkInfoView: public QWidget
{
    Q_OBJECT
public:
    explicit NetworkInfoView(QWidget* parent = nullptr);
    ~NetworkInfoView();
    void addKeyValue(QPair<QString, QString>);
private:
    void resizeKeyValTable();
    QTableView* keyValueTbl;
    QStandardItemModel* keyValModel;
    QTableView* indicatorInfoTbl;
};

#endif // NETWORKINFOVIEW_H
