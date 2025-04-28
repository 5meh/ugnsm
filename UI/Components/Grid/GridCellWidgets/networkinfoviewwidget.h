#ifndef NETWORKINFOVIEWWIDGET_H
#define NETWORKINFOVIEWWIDGET_H

#include "gridcellwidget.h"

#include <QFrame>
#include <QLabel>

QT_FORWARD_DECLARE_CLASS(QTableView)
QT_FORWARD_DECLARE_CLASS(QStandardItemModel);
QT_FORWARD_DECLARE_CLASS(QStandardItem)
class NetworkInfoModel;

class NetworkInfoViewWidget: public GridCellWidget
{
    Q_OBJECT
public:
    explicit NetworkInfoViewWidget(NetworkInfoModel* viewModel, QFrame* parent = nullptr);
    ~NetworkInfoViewWidget();

    void setViewModel(NetworkInfoModel* model);//TODO:mb add Q_PROPERTY
    const NetworkInfoModel* getModel()const;
    QString cellId() const override;
    void updateProperty(const QString& propertyName);
    QString getMac() const;
    Q_PROPERTY(bool updating READ isUpdating WRITE setUpdating NOTIFY updatingChanged)

    bool isUpdating() const;
    void setUpdating(bool newUpdating);

public slots:
    void updateNetworkInfoDisplay();
signals:
    void updatingChanged();

protected:
    void dragEnterEvent(QDragEnterEvent* event)override;
    void dragLeaveEvent(QDragLeaveEvent* event)override;
    void dropEvent(QDropEvent* event)override;

private:
    void updateStatusIndicator(QStandardItem* item, const QString& key,
                               const QString& value);

    void updateSpeedIndicators();
    //void resizeKeyValTable();
    void setupUI();
    void setKeyValueTbl();
    void addKeyValue(QPair<QString, QString>);
    void setupTableView();
    void connectViewModel();
    bool eventFilter(QObject* watched, QEvent* event) override;

    //TODO: mb remove constants
    //bool m_isTableDragging = false;
    //QPoint m_tableDragStartPos;
    NetworkInfoModel* m_viewModel;

    QTableView* keyValueTbl;
    QStandardItemModel* keyValModel;

    QLabel crownLbl;
    const QColor m_normalBorder = QColor(200, 200, 200);
    const QColor m_dragBorder = QColor(100, 150, 250);
    bool m_updating;
};

#endif // NETWORKINFOVIEWWIDGET_H
