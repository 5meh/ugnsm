#include "placeholdercellwidget.h"

#include <QStyle>

PlaceHolderCellWidget::PlaceHolderCellWidget(QWidget* parent)
    :GridCellWidget(parent)
{
    //setGeometry(sizeHint());
    setProperty("isPlaceholder", true);
}
