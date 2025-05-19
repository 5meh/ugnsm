#include "settingsdialog.h"

#include "../Core/Network/NetworkSortingStrategies/inetworksortstrategy.h"
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QPushButton>

#include "../../../Core/globalmanager.h"

SettingsDialog::SettingsDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUI();
    loadSettings();
    createConnections();
}

void SettingsDialog::setupUI()
{
    QFormLayout* layout = new QFormLayout(this);

    m_sortStrategyCombo = new QComboBox(this);
    m_sortStrategyCombo->addItem("Speed", "SpeedSortStrategy");
    m_sortStrategyCombo->addItem("Signal Strength", "SignalSortStrategy");
    m_sortStrategyCombo->addItem("Alphabetical", "AlphabeticalSortStrategy");

    m_updateIntervalSpin = new QSpinBox(this);
    m_updateIntervalSpin->setRange(500, 10000);
    m_updateIntervalSpin->setSingleStep(500);
    m_updateIntervalSpin->setSuffix(" ms");

    m_gridRowsSpin = new QSpinBox(this);
    m_gridRowsSpin->setRange(1, 10);
    m_gridColsSpin = new QSpinBox(this);
    m_gridColsSpin->setRange(1, 10);

    m_showBestNetworkWarningCheck = new QCheckBox(this);

    m_dataUnitsCombo = new QComboBox(this);
    m_dataUnitsCombo->addItems({"Bytes", "KB", "MB"});

    m_decimalPrecisionSpin = new QSpinBox(this);
    m_decimalPrecisionSpin->setRange(0, 4);

    m_autoRefreshCheck = new QCheckBox("Enable auto-refresh", this);

    m_bestNetworkCriteriaCombo = new QComboBox(this);
    m_bestNetworkCriteriaCombo->addItems({"Speed", "Signal Strength", "Combined Score"});

    layout->insertRow(2, "Data Units:", m_dataUnitsCombo);
    layout->insertRow(3, "Decimal Precision:", m_decimalPrecisionSpin);
    layout->insertRow(4, m_autoRefreshCheck);
    layout->insertRow(5, "Best Network Criteria:", m_bestNetworkCriteriaCombo);

    layout->addRow("Sorting Strategy:", m_sortStrategyCombo);
    layout->addRow("Update Interval:", m_updateIntervalSpin);
    layout->addRow("Grid Rows:", m_gridRowsSpin);
    layout->addRow("Grid Columns:", m_gridColsSpin);
    layout->addRow("Show Best Network Warning:", m_showBestNetworkWarningCheck);

    buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Apply | QDialogButtonBox::Cancel, this);

    layout->addRow(buttons);

    createConnections();
}

void SettingsDialog::createConnections()
{
    connect(buttons, &QDialogButtonBox::accepted, this, &SettingsDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &SettingsDialog::reject);
    connect(buttons->button(QDialogButtonBox::Apply), &QPushButton::clicked,
            this, &SettingsDialog::applySettings);
}

void SettingsDialog::loadSettings()
{
    m_sortStrategyCombo->setCurrentText(GlobalManager::settingsManager()->getSortStrategy());
    m_updateIntervalSpin->setValue(GlobalManager::settingsManager()->getUpdateInterval());
    m_gridRowsSpin->setValue(GlobalManager::settingsManager()->getGridRows());
    m_gridColsSpin->setValue(GlobalManager::settingsManager()->getGridCols());
    m_showBestNetworkWarningCheck->setChecked(GlobalManager::settingsManager()->getShowBestNetworkWarning());
    m_dataUnitsCombo->setCurrentText(GlobalManager::settingsManager()->getDataUnits());
    m_decimalPrecisionSpin->setValue(GlobalManager::settingsManager()->getDecimalPrecision());
    m_autoRefreshCheck->setChecked(GlobalManager::settingsManager()->getAutoRefresh());
    m_bestNetworkCriteriaCombo->setCurrentText(GlobalManager::settingsManager()->getBestNetworkCriteria());
}

void SettingsDialog::applySettings()
{
    GlobalManager::settingsManager()->setSortStrategy(m_sortStrategyCombo->currentText());
    GlobalManager::settingsManager()->setUpdateInterval(m_updateIntervalSpin->value());
    GlobalManager::settingsManager()->setGridRows(m_gridRowsSpin->value());
    GlobalManager::settingsManager()->setGridCols(m_gridColsSpin->value());
    GlobalManager::settingsManager()->setDataUnits(m_dataUnitsCombo->currentData().toString());
    GlobalManager::settingsManager()->setDecimalPrecision(m_decimalPrecisionSpin->value());
    GlobalManager::settingsManager()->setShowBestNetworkWarning(m_showBestNetworkWarningCheck->isChecked());
    GlobalManager::settingsManager()->setAutoRefresh(m_autoRefreshCheck->isChecked());
    GlobalManager::settingsManager()->setBestNetworkCriteria(m_bestNetworkCriteriaCombo->currentData().toString());
}

void SettingsDialog::accept()
{
    applySettings();
    QDialog::accept();
}

void SettingsDialog::reject()
{
    loadSettings();
    QDialog::reject();
}
