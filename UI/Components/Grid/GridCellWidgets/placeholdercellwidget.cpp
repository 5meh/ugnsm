#include "placeholdercellwidget.h"

#include <QStyle>

PlaceHolderCellWidget::PlaceHolderCellWidget(QWidget* parent)
    :GridCellWidget(parent)
{
    setFixedSize(m_widgetSize);
    //setStyleSheet("background-color: #F0F0F0; border: 2px dashed #AAAAAA; border-radius: 8px;");
    setProperty("isPlaceholder", true);
    style()->unpolish(this);//TODO:mb remove
    style()->polish(this);
}

QString PlaceHolderCellWidget::cellId() const
{
    return objectName().isEmpty() ? "placeholder" : objectName();
}
