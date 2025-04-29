#include "placeholdercellwidget.h"

#include <QStyle>

PlaceHolderCellWidget::PlaceHolderCellWidget(QWidget* parent)
    :GridCellWidget(parent)
{
    setFixedSize(sizeHint());
    setProperty("isPlaceholder", true);
}
