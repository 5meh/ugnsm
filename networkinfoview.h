#ifndef NETWORKINFOVIEW_H
#define NETWORKINFOVIEW_H

#include <QObject>
#include <QWidget>
//#include <QTableWidget>
QT_FORWARD_DECLARE_CLASS(QTableView)

class NetworkInfoView: public QWidget
{
    Q_OBJECT
public:
    NetworkInfoView();
private:
    QTableView* _keyValueTbl;
    QTableView* _indicatoRInfoTbl;

};

#endif // NETWORKINFOVIEW_H
