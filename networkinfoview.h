#ifndef NETWORKINFOVIEW_H
#define NETWORKINFOVIEW_H

#include <QObject>
#include <QWidget>
//#include <QTableWidget>
QT_FORWARD_DECLARE_CLASS(QTableView)
QT_FORWARD_DECLARE_CLASS(QStandardItemModel);
#include <QAbstractItemView>

class NetworkInfoView: public QWidget
{
    Q_OBJECT
public:
    explicit NetworkInfoView(QWidget* parent = nullptr);
    ~NetworkInfoView();
    void addKeyValue(QPair<QString, QString>);

signals:
    void swapRequested(QWidget *source, QWidget *target);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
private:
    void resizeKeyValTable();
    QTableView* keyValueTbl;
    QStandardItemModel* keyValModel;
    QTableView* indicatorInfoTbl;
    QPoint dragStartPos;
};

#endif // NETWORKINFOVIEW_H
