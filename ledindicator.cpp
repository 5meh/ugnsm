#include "ledindicator.h"
#include <QMetaEnum>

LedIndicator::LedIndicator(QWidget* parent):
    QFrame(parent), currentStatus(Status::Red)
{
    setFixedSize(20, 20);
    setFrameShape(QFrame::StyledPanel);
    setFrameShadow(QFrame::Raised);
    updateColor();
}

void LedIndicator::setStatus(Status status)
{
    if(currentStatus != status)
    {
        currentStatus = status;
        updateColor();
    }
}

QString LedIndicator::statusToString(Status color)
{
    QMetaEnum metaEnum = QMetaEnum::fromType<LedIndicator::Status>();
    return QString::fromLatin1(metaEnum.valueToKey(static_cast<int>(color)));
}

void LedIndicator::updateColor()
{
    QString color;
    switch(currentStatus)
    {
        case Status::Red: color = "red"; break;
        case Status::Yellow: color = "yellow"; break;
        case Status::Green: color = "green"; break;
    }
    setStyleSheet(QString("background-color: %1; border-radius: 10px;").arg(color));
}
