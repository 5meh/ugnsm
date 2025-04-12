#ifndef PLACEHOLDERCELLWIDGET_H
#define PLACEHOLDERCELLWIDGET_H

#include "gridcellwidget.h"

class PlaceHolderCellWidget: public GridCellWidget
{
    Q_OBJECT
public:
    PlaceHolderCellWidget();
    ~PlaceHolderCellWidget() = default;

    QString cellId() const override;
};

#endif // PLACEHOLDERCELLWIDGET_H
