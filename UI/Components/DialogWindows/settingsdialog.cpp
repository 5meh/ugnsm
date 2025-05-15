#include "settingsdialog.h"

#include "../Core/Network/NetworkSortingStrategies/inetworksortstrategy.h"
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QPushButton>

#include "../../Core/Settings/settingsmanager.h"

SettingsDialog::SettingsDialog(QWidget* parent)
    : QDialog(parent),
    m_settings("YourCompany", "NetworkDashboard")
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

    m_unitsCombo = new QComboBox(this);
    m_unitsCombo->addItems({"Bytes", "KB", "MB"});

    m_decimalPrecisionSpin = new QSpinBox(this);
    m_decimalPrecisionSpin->setRange(0, 4);

    m_autoRefreshCheck = new QCheckBox("Enable auto-refresh", this);

    m_bestNetworkCriteriaCombo = new QComboBox(this);
    m_bestNetworkCriteriaCombo->addItems({"Speed", "Signal Strength", "Combined Score"});

    layout->insertRow(2, "Data Units:", m_unitsCombo);
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
    m_settings.beginGroup("Settings");
    m_sortStrategyCombo->setCurrentText(
        m_settings.value("SortStrategy", "Speed").toString());
    m_updateIntervalSpin->setValue(
        m_settings.value("UpdateInterval", 2000).toInt());
    m_gridRowsSpin->setValue(
        m_settings.value("GridRows", 3).toInt());
    m_gridColsSpin->setValue(
        m_settings.value("GridCols", 3).toInt());
    m_showBestNetworkWarningCheck->setChecked(
        m_settings.value("ShowBestNetworkWarning", true).toBool());
    m_unitsCombo->setCurrentText(
        m_settings.value("DataUnits", "MB").toString());
    m_decimalPrecisionSpin->setValue(
        m_settings.value("DecimalPrecision", 2).toInt());
    m_autoRefreshCheck->setChecked(
        m_settings.value("AutoRefresh", true).toBool());
    m_bestNetworkCriteriaCombo->setCurrentText(
        m_settings.value("BestNetworkCriteria", "Combined Score").toString());
    m_settings.endGroup();
}

void SettingsDialog::applySettings()
{
    m_settings.beginGroup("Settings");
    m_settings.setValue("SortStrategy", m_sortStrategyCombo->currentText());
    m_settings.setValue("UpdateInterval", m_updateIntervalSpin->value());
    m_settings.setValue("GridRows", m_gridRowsSpin->value());
    m_settings.setValue("GridCols", m_gridColsSpin->value());
    m_settings.setValue("ShowBestNetworkWarning",
                        m_showBestNetworkWarningCheck->isChecked());
    m_settings.endGroup();

    emit settingsChanged();
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
