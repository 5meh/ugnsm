#ifndef NETWORKINFOVIEWWIDGET_H
#define NETWORKINFOVIEWWIDGET_H

#include "gridcellwidget.h"

#include <QFrame>
#include <QLabel>

QT_FORWARD_DECLARE_CLASS(QTableView)
//QT_FORWARD_DECLARE_CLASS(QStandardItemModel);
QT_FORWARD_DECLARE_CLASS(QStandardItem)
class NetworkInfoModel;

class NetworkInfoViewWidget: public GridCellWidget
{
    Q_OBJECT
public:
    explicit NetworkInfoViewWidget(NetworkInfoModel* viewModel, QFrame* parent = nullptr);
    ~NetworkInfoViewWidget();

    QString cellId() const override;
    QString getMac() const;

public slots:
    void updateNetworkInfoDisplay();
protected:
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

    const QSize m_widgetSize = QSize(400, 300);//TODO: mb remove constants
    //bool m_isTableDragging = false;
    //QPoint m_tableDragStartPos;
    NetworkInfoModel* m_viewModel;

    QTableView* keyValueTbl;
    QStandardItemModel* keyValModel;

    QLabel crownLbl;
    //bool m_isDragging = false;
    //QPoint dragStartPos;
    const QColor m_normalBorder = QColor(200, 200, 200);
    const QColor m_dragBorder = QColor(100, 150, 250);
};

#endif // NETWORKINFOVIEWWIDGET_H
