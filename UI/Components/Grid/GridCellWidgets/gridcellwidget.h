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

    // Derived classes must provide a unique cell identifier (e.g., "row,col")
    virtual QString cellId() const = 0;

signals:
    void swapRequested(GridCellWidget* source, GridCellWidget* target);
    void dragInitiated(GridCellWidget* source);
    void dropReceived(GridCellWidget* target);

protected:
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void dragEnterEvent(QDragEnterEvent* event) override;
    virtual void dropEvent(QDropEvent* event) override;
    virtual void dragLeaveEvent(QDragLeaveEvent* event) override;

    const QSize m_widgetSize = QSize(400, 300);
    //QPoint m_dragStartPosition;
};

#endif // GRIDCELLWIDGET_H
