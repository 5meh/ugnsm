#ifndef GRIDCELLWIDGET_H
#define GRIDCELLWIDGET_H

#include <QFrame>

QT_FORWARD_DECLARE_CLASS(QMouseEvent)
QT_FORWARD_DECLARE_CLASS(QDragEnterEvent)
QT_FORWARD_DECLARE_CLASS(QDropEvent)
QT_FORWARD_DECLARE_CLASS(QDragLeaveEvent)

class GridCellWidget : public QFrame
{
    Q_OBJECT
public:
    explicit GridCellWidget(QWidget* parent = nullptr);
    virtual ~GridCellWidget();

    QPoint getGridIndex() const;
    void setGridIndex(QPoint newGridIndex);
    void highlightCell();
    void clearHighlight();

signals:
    void swapRequested(QPoint source, QPoint target);
    void gridIndexChanged();

protected:
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void dragEnterEvent(QDragEnterEvent* event) override;
    virtual void dropEvent(QDropEvent* event) override;
    virtual void dragLeaveEvent(QDragLeaveEvent* event) override;

    QPoint m_gridIndex;
    QPoint m_dragStartPos;
private:
    Q_PROPERTY(QPoint gridIndex READ getGridIndex WRITE setGridIndex NOTIFY gridIndexChanged FINAL)
};

#endif // GRIDCELLWIDGET_H
