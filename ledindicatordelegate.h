#ifndef LEDINDICATORDELEGATE_H
#define LEDINDICATORDELEGATE_H

#include <QStyledItemDelegate>

class LedIndicatorDelegate: public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit LedIndicatorDelegate(QObject* parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
private:
    int determineStateFromValue(const QVariant &value) const;
    QColor getColorForState(int state) const;
    void animateColor(QColor &color) const;
    void drawLED(QPainter *painter, const QStyleOptionViewItem &option, const QColor &color) const;
};

#endif // LEDINDICATORDELEGATE_H
