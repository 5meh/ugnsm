#ifndef PLACEHOLDERCELLWIDGET_H
#define PLACEHOLDERCELLWIDGET_H

#include "gridcellwidget.h"

class PlaceHolderCellWidget: public GridCellWidget
{
    Q_OBJECT
public:
    PlaceHolderCellWidget(QWidget* parent = nullptr);
    ~PlaceHolderCellWidget() = default;
};

#endif // PLACEHOLDERCELLWIDGET_H
