#include "ledindicatordelegate.h"
#include "ledindicator.h"

#include <QTimer>
#include <QTableView>
#include <QDateTime>
#include <QPainter>

LedIndicatorDelegate::LedIndicatorDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{

}

void LedIndicatorDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.column() == 2)
    {
        // Get LED state directly from model
        int state = index.data(Qt::UserRole).toInt();

        if (state >= 0)
        {
            QColor color = getColorForState(state);

            painter->save();
            painter->setRenderHint(QPainter::Antialiasing);

            // Create LED circle
            int size = qMin(option.rect.width(), option.rect.height()) - 6;
            QRect ledRect(option.rect.center().x() - size/2,
                          option.rect.center().y() - size/2,
                          size, size);

            // Draw LED circle
            painter->setPen(QPen(Qt::black, 1));
            painter->setBrush(color);
            painter->drawEllipse(ledRect);

            painter->restore();
            return;
        }
    }

    // Default painting for other cells
    QStyledItemDelegate::paint(painter, option, index);
}

QSize LedIndicatorDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.column() == 2)
    {
        return QSize(40, 20);
    }
    return QStyledItemDelegate::sizeHint(option, index);
}

int LedIndicatorDelegate::determineStateFromValue(const QVariant &value) const
{
    //TODO: mb later add yellow processing
    QString valueStr(value.toString());
    if(valueStr.compare("True") == 0)
        return 2;
    return 0;
}

QColor LedIndicatorDelegate::getColorForState(int state) const
{
    switch (state)
    {
        case 0:  return Qt::red;
        case 1:  return Qt::yellow;
        case 2:  return Qt::green;
        default: return Qt::gray;
    }
}

void LedIndicatorDelegate::animateColor(QColor &color) const
{
    qint64 ms = QDateTime::currentMSecsSinceEpoch();
    qreal progress = (ms % 1000) / 1000.0;
    qreal alpha = 0.5 + 0.5 * qSin(progress * 2 * M_PI);
    color.setAlphaF(alpha);
}

void LedIndicatorDelegate::drawLED(QPainter *painter, const QStyleOptionViewItem &option, const QColor &color) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(Qt::NoPen);
    painter->setBrush(color);
    painter->drawRect(option.rect);
    // painter->drawEllipse(option.rect.center(),
    //                      option.rect.height()/2 - 2,
    //                      option.rect.height()/2 - 2);
    painter->restore();
}
