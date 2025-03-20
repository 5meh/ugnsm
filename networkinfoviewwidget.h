#ifndef NETWORKINFOVIEWWIDGET_H
#define NETWORKINFOVIEWWIDGET_H

#include <QObject>
#include <QFrame>
QT_FORWARD_DECLARE_CLASS(QTableView)
QT_FORWARD_DECLARE_CLASS(QStandardItemModel);

class NetworkInfo;
//#include <QAbstractItemView>

class NetworkInfoViewWidget: public QFrame
{
    Q_OBJECT
public:
    explicit NetworkInfoViewWidget(QWidget* parent = nullptr);
    NetworkInfoViewWidget(NetworkInfo* info, QWidget* parent = nullptr);
    ~NetworkInfoViewWidget();
    QString getMac() const;
signals:
    void swapRequested(QWidget *source, QWidget *target);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
private:
    void resizeKeyValTable();
    void setupUI();
    void setKeyValueTbl();
    void addKeyValue(QPair<QString, QString>);
    NetworkInfo* m_info;
    QTableView* keyValueTbl;
    QStandardItemModel* keyValModel;
    QTableView* indicatorInfoTbl;
    QPoint dragStartPos;
};

#endif // NETWORKINFOVIEWWIDGET_H
