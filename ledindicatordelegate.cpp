#include "ledindicatordelegate.h"
#include "ledindicator.h"

LedIndicatorDelegate::LedIndicatorDelegate(QObject* parent) : QStyledItemDelegate(parent)
{
}

QWidget *LedIndicatorDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex&) const
{
    return new LedIndicator(parent);
}

void LedIndicatorDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    LedIndicator* led = static_cast<LedIndicator*>(editor);
    int value = index.model()->data(index, Qt::EditRole).toInt();
    led->setStatus(static_cast<LedIndicator::Status>(value));
}

void LedIndicatorDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    LedIndicator* led = static_cast<LedIndicator*>(editor);
    model->setData(index, led->property("status").toInt(), Qt::EditRole);
}
