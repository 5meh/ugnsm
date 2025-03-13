#include "networkinfoview.h"
#include <QLabel>
#include <QTableView>
#include <QVBoxLayout>
#include <QAbstractTableModel>
#include <QStandardItemModel>
#include <QHeaderView>

NetworkInfoView::NetworkInfoView(QWidget* parent):
    QWidget(parent),
    keyValueTbl(new QTableView(this)),
    indicatorInfoTbl(new QTableView(this))
{
    setLayout(new QVBoxLayout(this));
    keyValueTbl->setEditTriggers(QAbstractItemView::NoEditTriggers);
    //keyValueTbl->resizeColumnsToContents();
    keyValueTbl->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    keyValueTbl->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    keyValueTbl->setFrameShape(QFrame::NoFrame);




    //keyValueTbl->resizeColumnsToContents();
    //keyValueTbl->resizeRowsToContents();

    keyValueTbl->horizontalHeader()->setVisible(false);
    keyValueTbl->verticalHeader()->setVisible(false);
    keyValueTbl->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);


    indicatorInfoTbl->setEditTriggers(QAbstractItemView::NoEditTriggers);
    indicatorInfoTbl->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    indicatorInfoTbl->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    indicatorInfoTbl->horizontalHeader()->setVisible(false);
    indicatorInfoTbl->verticalHeader()->setVisible(false);


    keyValModel = new QStandardItemModel(this);
    keyValueTbl->setModel(keyValModel);

    //TODO:add indicator inicialization

    layout()->addWidget(keyValueTbl);
    layout()->addWidget(indicatorInfoTbl);
    //TODO: remove later
    indicatorInfoTbl->hide();
}

NetworkInfoView::~NetworkInfoView()
{

}

void NetworkInfoView::addKeyValue(QPair<QString, QString> keyVal)
{
    QList<QStandardItem*> newRow;
    newRow << new QStandardItem(QString(keyVal.first)) << new QStandardItem(QString(keyVal.second));
    keyValModel->appendRow(newRow);
    resizeKeyValTable();
}

void NetworkInfoView::resizeKeyValTable()
{
    //TODO:fix calculation of size
    int totalWidth = 0;
    // Add width of vertical header even if hidden (it may contribute if not set to 0)
    totalWidth += keyValueTbl->verticalHeader()->width();
    for (int col = 0; col < keyValModel->columnCount(); ++col) {
        totalWidth += keyValueTbl->columnWidth(col);
    }

    int totalHeight = 0;
    // Add height of horizontal header even if hidden
    totalHeight += keyValueTbl->horizontalHeader()->height();
    for (int row = 0; row < keyValModel->rowCount(); ++row) {
        totalHeight += keyValueTbl->rowHeight(row);
    }

    // Set fixed size based on calculated dimensions
    keyValueTbl->setFixedSize(totalWidth*5, totalHeight);
}
