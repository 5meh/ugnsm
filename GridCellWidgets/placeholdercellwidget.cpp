#include "placeholdercellwidget.h"

PlaceHolderCellWidget::PlaceHolderCellWidget()
{
    setFixedSize(m_widgetSize);
    setStyleSheet("background-color: #F0F0F0; border: 2px dashed #AAAAAA; border-radius: 8px;");
    setProperty("isPlaceholder", true);
}

QString PlaceHolderCellWidget::cellId() const
{
    return objectName().isEmpty() ? "placeholder" : objectName();
}
