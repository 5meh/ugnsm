#include "gridviewmanager.h"
#include "../GridCellWidgets/gridcellwidget.h"
#include "../GridCellWidgets/networkinfoviewwidget.h"
#include "../GridCellWidgets/placeholdercellwidget.h"
#include "../Core/Network/Information/networkinfomodel.h"
#include "../Core/globalmanager.h"

#include <QMimeData>
#include <QDrag>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QPainter>
#include <QDebug>
#include <QStyle>

GridViewManager::GridViewManager(QWidget* parent)
    : QWidget(parent),
    m_gridLayout(new QGridLayout(this))
{
    updateUI = [](QWidget* widget)
    {
        widget->style()->unpolish(widget);
        widget->style()->polish(widget);
    };
    m_gridLayout->setSpacing(10);
    m_gridLayout->setContentsMargins(10, 10, 10, 10);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    setLayout(m_gridLayout);
    setAcceptDrops(true);
}

GridViewManager::~GridViewManager()
{
    clearGrid();
}

void GridViewManager::setGridSize(int rows, int cols)
{
    clearGrid();
    m_cells.resize(rows);

    m_gridLayout->setSpacing(10);
    m_gridLayout->setContentsMargins(10, 10, 10, 10);

    for(int row = 0; row < rows; ++row)
    {
        m_cells[row].resize(cols);
        for(int col = 0; col < cols; ++col)
        {
            PlaceHolderCellWidget* cell = new PlaceHolderCellWidget();
            setCell(QPoint(row, col), cell);
        }
    }
}

void GridViewManager::setCell(QPoint indx, GridCellWidget* widget)
{
    if(indx.x() < 0 || indx.x() >= m_cells.size() || indx.y() < 0 || indx.y() >= m_cells[indx.x()].size())//TODO:mb remove range check everywhere?
        return;

    GridCellWidget* oldWidget = m_cells[indx.x()][indx.y()];
    if(oldWidget)
    {
        m_gridLayout->removeWidget(oldWidget);
        oldWidget->deleteLater();
    }

    widget->setGridIndex(indx);

    m_gridLayout->addWidget(widget, indx.x(), indx.y());
    m_cells[indx.x()][indx.y()] = widget;

    if (indx == QPoint(0, 0))
    {
        widget->setProperty("bestNetwork", true);

        if (!isPlaceholder(widget))
            highlightCell(0, 0);
        else
            clearHighlight(0, 0);
    }
    else
        widget->setProperty("bestNetwork", false);

    connect(widget, &GridCellWidget::swapRequested,
            this, &GridViewManager::handleSwapRequested);

    widget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    //widget->setMinimumSize(200, 150); // Set minimum size to match network widgets

    widget->setFocusPolicy(Qt::NoFocus);
    widget->setAttribute(Qt::WA_TransparentForMouseEvents, false);
}

void GridViewManager::updateCell(QPoint indx, QSharedPointer<NetworkInfoModel> model)
{
    if (indx.x() < 0 || indx.x() >= gridRows() || indx.y() < 0 || indx.y() >= gridCols())
        return;

    if (model.isNull())
    {
        clearCell(indx.x(), indx.y());
        return;
    }

    GridCellWidget* current = cellAt(indx.x(), indx.y());
    setUpdatesEnabled(false);

    if (!model.isNull())
    {
        NetworkInfoViewWidget* networkWidget = qobject_cast<NetworkInfoViewWidget*>(current);
        if (networkWidget)
        {
            // Update existing widget if model is different
            if (networkWidget->getModel().data() != model.data())//TODO:add proper comprasion, and check if it's happen "higher"
            {
                networkWidget->setViewModel(model);
            }
        }
        else
        {
            // Replace placeholder with new NetworkInfoViewWidget
            auto* newWidget = createCellWidgetForModel(model);
            newWidget->setGridIndex(indx);
            setCell(indx, newWidget);
        }
    }
    setUpdatesEnabled(true);
}

void GridViewManager::clearCell(int row, int col)
{
    if(auto* current = cellAt(row, col))
    {
        if(current->metaObject()->className() != PlaceHolderCellWidget::staticMetaObject.className())
        {
            setCell(QPoint(row, col), new PlaceHolderCellWidget(this));
        }
    }
}

void GridViewManager::setUpdatesEnabled(bool enable)
{
    if (m_updatesEnabled == enable)
        return;

    m_updatesEnabled = enable;
    emit pauseGridUpdates(!enable);
}

GridCellWidget* GridViewManager::cellAt(int row, int col) const
{
    if(row >= 0 && row < m_cells.size() &&
        col >= 0 && col < m_cells[row].size())
    {
        return m_cells[row][col];
    }
    return nullptr;//TODO:it's imposible
}

void GridViewManager::handleSwapRequested(QPoint source, QPoint target)
{
    if (!m_updatesEnabled)
    {
        // If updates are paused, queue the swap request
        QTimer::singleShot(0, this, [this, source, target]() {
            handleSwapRequested(source, target);
        });
        return;
    }

    handleSwapRequestedImpl(source, target);
}

void GridViewManager::handleSwapRequestedImpl(QPoint source, QPoint target)
{
    if (source.x() < 0 || source.x() >= gridRows() || source.y() < 0 || source.y() >= gridCols() ||
        target.x() < 0 || target.x() >= gridRows() || target.y() < 0 || target.y() >= gridCols())
        return;

    GridCellWidget* sourceWidget = cellAt(source.x(), source.y());
    GridCellWidget* targetWidget = cellAt(target.x(), target.y());

    // Flag to track if we need to show a dialog
    bool showDialog = false;
    QString dialogId;
    QString title;
    QString message;
    QString checkboxText = "Do not show this message again";

    // Check if swap involves best network position (0,0)
    if (source == QPoint(0,0) || target == QPoint(0,0))
    {
        GridCellWidget* bestNetworkCell = nullptr;
        GridCellWidget* otherCell = nullptr;

        // Identify which cell is at (0,0)
        if (source == QPoint(0,0))
        {
            bestNetworkCell = sourceWidget;
            otherCell = targetWidget;
        }
        else
        {
            bestNetworkCell = targetWidget;
            otherCell = sourceWidget;
        }

        // Case 1: Swap between two networks (one is best network)
        if (!isPlaceholder(bestNetworkCell) && !isPlaceholder(otherCell))
        {
            showDialog = true;
            dialogId = "SwapWarning";
            title = "Best Network Swap Warning";
            message = "You are trying to swap the best network.\nDo you want to continue?";
        }
        // Case 2: Swap best network with placeholder
        else if (!isPlaceholder(bestNetworkCell) && isPlaceholder(otherCell))
        {
            showDialog = true;
            dialogId = "CancelNetworkWarning";
            title = "Cancel Network Connection";
            message = "You are disconnecting the best network.\nDo you want to continue?";
        }
        // Case 3: Swap placeholder at (0,0) with network
        else if (isPlaceholder(bestNetworkCell) && !isPlaceholder(otherCell))
        {
            showDialog = true;
            dialogId = "SetActiveWarning";
            title = "Set Active Network";
            message = "You are setting this network as active.\nDo you want to continue?";
        }
    }

    if (showDialog)
    {

        auto result = GlobalManager::messageBoxManager()->showDialog(
            dialogId,
            title,
            message,
            checkboxText
            );


        if (result == QMessageBox::Yes)
        {
            performSwap(source, target);
            return;
        }

    }

    performSwap(source, target);
}

void GridViewManager::clearGrid()
{
    for(auto& row : m_cells)
    {
        for(auto cell : row)
        {
            m_gridLayout->removeWidget(cell);
            cell->deleteLater();
        }
    }
    m_cells.clear();
}

void GridViewManager::highlightCell(int row, int col)
{
    if (m_highlightedCell)
        m_highlightedCell->clearHighlight();

    GridCellWidget* cell = cellAt(row, col);
    if (cell && !isPlaceholder(cell))
    {
        cell->highlightCell();
        m_highlightedCell = cell;
    }
    else
        m_highlightedCell = nullptr;
}

void GridViewManager::clearHighlight(int row, int col)
{
    if (m_highlightedCell)
    {
        m_highlightedCell->clearHighlight();
        m_highlightedCell = nullptr;
    }
}

QPoint GridViewManager::getCellIndexFromPos(const QPoint& indx)
{
    if(indx.x() >= 0 && indx.x() < gridRows() &&
        indx.y() >= 0 && indx.y() < gridCols())
    {
        return indx;
    }
    return QPoint(-1, -1);
}

GridCellWidget* GridViewManager::createCellWidgetForModel(QSharedPointer<NetworkInfoModel> model)
{
    NetworkInfoViewWidget* widget = new NetworkInfoViewWidget(model);
    if(widget->size().width() > m_maxCellSize.width() &&  widget->size().height() > m_maxCellSize.height())
    {
        m_maxCellSize = widget->size();
        //applyUniformCellSize();//TODO:fix later

    }
    connect(model.get(), &NetworkInfoModel::propertyChanged,
            widget, &NetworkInfoViewWidget::updateProperty,
            Qt::QueuedConnection);

    return widget;
}

bool GridViewManager::isPlaceholder(GridCellWidget *widget) const
{
    return qobject_cast<PlaceHolderCellWidget*>(widget) != nullptr;
}

void GridViewManager::performSwap(QPoint source, QPoint target)
{
    GridCellWidget* sourceWidget = cellAt(source.x(), source.y());
    GridCellWidget* targetWidget = cellAt(target.x(), target.y());

    if (!sourceWidget || !targetWidget)
        return;

    sourceWidget->blockSignals(true);
    targetWidget->blockSignals(true);

    m_gridLayout->removeWidget(sourceWidget);
    m_gridLayout->removeWidget(targetWidget);

    m_gridLayout->addWidget(sourceWidget, target.x(), target.y());
    m_gridLayout->addWidget(targetWidget, source.x(), source.y());

    std::swap(m_cells[source.x()][source.y()], m_cells[target.x()][target.y()]);

    sourceWidget->setGridIndex(target);
    targetWidget->setGridIndex(source);

    if (source == QPoint(0, 0) || target == QPoint(0, 0))
    {
        GridCellWidget* bestCell = cellAt(0, 0);
        bestCell->setProperty("bestNetwork", true);
        updateUI(bestCell);
        //bestCell->style()->unpolish(bestCell);
        //bestCell->style()->polish(bestCell);

        if (auto* placeholder = qobject_cast<PlaceHolderCellWidget*>(bestCell))
        {
            placeholder->setGridIndex(QPoint(0, 0)); // Force text update
        }

        if (!isPlaceholder(bestCell))
            highlightCell(0, 0);
        else
            clearHighlight(0, 0);
    }

    sourceWidget->blockSignals(false);
    targetWidget->blockSignals(false);

    emit cellSwapRequestToDataManager(source, target);
}

void GridViewManager::applyUniformCellSize()
{
    for (auto& row : m_cells)
        for (auto* cell : row)
            cell->setFixedSize(m_maxCellSize);
}
