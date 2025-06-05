#ifndef PLACEHOLDERCELLWIDGET_H
#define PLACEHOLDERCELLWIDGET_H

#include "gridcellwidget.h"

QT_FORWARD_DECLARE_CLASS(QLabel)

class PlaceHolderCellWidget: public GridCellWidget
{
    Q_OBJECT
public:
    PlaceHolderCellWidget(QWidget* parent = nullptr);
    ~PlaceHolderCellWidget() = default;
    void setGridIndex(QPoint newGridIndex);
protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;
    void dropEvent(QDropEvent* event) override;
private:
    QLabel* m_infoLabel;
};

#endif // PLACEHOLDERCELLWIDGET_H
