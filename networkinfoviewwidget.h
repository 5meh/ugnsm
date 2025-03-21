#ifndef NETWORKINFOVIEWWIDGET_H
#define NETWORKINFOVIEWWIDGET_H

#include <QFrame>
#include <QLabel>

QT_FORWARD_DECLARE_CLASS(QTableView)
QT_FORWARD_DECLARE_CLASS(QStandardItemModel);
QT_FORWARD_DECLARE_CLASS(QStandardItem)
class NetworkInfo;

class NetworkInfoViewWidget: public QFrame
{
    Q_OBJECT
public:
    explicit NetworkInfoViewWidget(QWidget* parent = nullptr);
    NetworkInfoViewWidget(NetworkInfo* info, QWidget* parent = nullptr);
    ~NetworkInfoViewWidget();
    QString getMac() const;
    void setStyleWhenSelected();
    void unsetStyleWhenSelected();
signals:
    void swapRequested(QWidget* source, QWidget* target);
public slots:
    void updateNetworkInfoDisplay();
protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;
private:
    void updateStatusIndicator(QStandardItem* item, const QString& key,
                               const QString& value);
    void resizeKeyValTable();
    void setupUI();
    void setKeyValueTbl();
    void addKeyValue(QPair<QString, QString>);

    const QSize m_widgetSize = QSize(300, 180);
    bool m_isTableDragging = false;
    QPoint m_tableDragStartPos;
    NetworkInfo* m_info;
    QTableView* keyValueTbl;
    QStandardItemModel* keyValModel;
    QTableView* indicatorInfoTbl;
    QLabel crownLbl;
    QPoint dragStartPos;
};

#endif // NETWORKINFOVIEWWIDGET_H
