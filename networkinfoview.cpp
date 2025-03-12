#include "networkinfoview.h"
#include <QLabel>
#include <QTableView>
#include <QVBoxLayout>
#include <QAbstractTableModel>

NetworkInfoView::NetworkInfoView():
    _keyValueTbl(new QTableView(this)),
    _indicatoRInfoTbl(new QTableView(this))
{
    setLayout(new QVBoxLayout(this));
    _keyValueTbl->setEditTriggers(QAbstractItemView::NoEditTriggers);
    _indicatoRInfoTbl->setEditTriggers(QAbstractItemView::NoEditTriggers);

    layout()->addWidget(_keyValueTbl);
    layout()->addWidget(_indicatoRInfoTbl);
}
