#ifndef PLACEHOLDERCELLWIDGET_H
#define PLACEHOLDERCELLWIDGET_H

#include "gridcellwidget.h"

class PlaceHolderCellWidget: public GridCellWidget
{
    Q_OBJECT
public:
    PlaceHolderCellWidget(QWidget* parent = nullptr);
    ~PlaceHolderCellWidget() = default;
protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;
    void dropEvent(QDropEvent* event) override;
};

#endif // PLACEHOLDERCELLWIDGET_H
