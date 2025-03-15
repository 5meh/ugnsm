#include "ledindicatordelegate.h"
#include "ledindicator.h"

#include <QTimer>
#include <QTableView>
#include <QDateTime>
#include <QPainter>

LedIndicatorDelegate::LedIndicatorDelegate(QObject* parent) : QStyledItemDelegate(parent)
{
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this]()
            {
                if (auto view = qobject_cast<QTableView*>(this->parent()))
                {
                    view->viewport()->update();
                }
            });
    timer->start(50);
}

void LedIndicatorDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.column() == 2)
    {
        QModelIndex firstColIndex = index.sibling(index.row(), 0);
        QVariant value = firstColIndex.data(Qt::DisplayRole);

        // Only draw if we have valid data
        if (value.isValid())
        {
            // Convert first column's value to LED state
            int state = determineStateFromValue(value);

            QColor color = getColorForState(state);

            // Apply animation if needed
            if (state == 1)// Yellow state
            {
                animateColor(color);
            }

            drawLED(painter, option, color);
            // Animation logic for yellow state
            if (state == 1)
            {
                qint64 ms = QDateTime::currentMSecsSinceEpoch();
                qreal progress = (ms % 1000) / 1000.0;
                qreal alpha = 0.5 + 0.5 * qSin(progress * 2 * M_PI);
                color.setAlphaF(alpha);
            }

            // Draw LED
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing);
            painter->setPen(Qt::NoPen);
            painter->setBrush(color);
            painter->drawEllipse(option.rect.center(),
                                 option.rect.height()/2 - 2,
                                 option.rect.height()/2 - 2);
            painter->restore();
        }
        else
        {
            // Leave cell empty by letting base class handle it
            QStyledItemDelegate::paint(painter, option, index);
        }

    }
}

QSize LedIndicatorDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.column() == 2)
    {
        return QSize(40, 20); // Fixed size for LED column
    }
    return QStyledItemDelegate::sizeHint(option, index);
}

int LedIndicatorDelegate::determineStateFromValue(const QVariant &value) const
{
    //TODO: make proper check
    QString numericValue(value.toString());
    return 1;
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
    painter->drawEllipse(option.rect.center(),
                         option.rect.height()/2 - 2,
                         option.rect.height()/2 - 2);
    painter->restore();
}
