#ifndef NETWORKINFOVIEWWIDGET_H
#define NETWORKINFOVIEWWIDGET_H

#include <QFrame>
#include <QLabel>

QT_FORWARD_DECLARE_CLASS(QTableView)
QT_FORWARD_DECLARE_CLASS(QStandardItemModel);
QT_FORWARD_DECLARE_CLASS(QStandardItem)
//class NetworkInfo;
class NetworkInfoViewModel;

class NetworkInfoViewWidget: public QFrame
{
    Q_OBJECT
public:
    explicit NetworkInfoViewWidget(NetworkInfoViewModel* viewModel, QWidget* parent = nullptr);
    ~NetworkInfoViewWidget();
    QString getMac() const;
signals:
    void swapRequested(QWidget* source, QWidget* target);

    void dragInitiated(QWidget* source);
    void dropReceived(QWidget* target);
public slots:
    void updateNetworkInfoDisplay();
protected:
    //bool eventFilter(QObject* watched, QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
private:
    void updateStatusIndicator(QStandardItem* item, const QString& key,
                               const QString& value);
    //void resizeKeyValTable();
    void setupUI();
    void setKeyValueTbl();
    void addKeyValue(QPair<QString, QString>);
    void setupTableView();
    void connectViewModel();

    QString formatSpeed(quint64 bytes) const;

    const QSize m_widgetSize = QSize(300, 180);
    bool m_isTableDragging = false;
    QPoint m_tableDragStartPos;
    //NetworkInfo* m_info;
    NetworkInfoViewModel* m_viewModel;

    QTableView* keyValueTbl;
    QStandardItemModel* keyValModel;
    //QTableView* indicatorInfoTbl;
    QLabel crownLbl;
    bool m_isDragging = false;
    QPoint dragStartPos;
    const QColor m_normalBorder = QColor(200, 200, 200);
    const QColor m_dragBorder = QColor(100, 150, 250);
};

#endif // NETWORKINFOVIEWWIDGET_H
