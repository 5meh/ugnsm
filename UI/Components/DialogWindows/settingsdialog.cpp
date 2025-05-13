#include "settingsdialog.h"

#include "../Core/Network/NetworkSortingStrategies/inetworksortstrategy.h"
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QFormLayout>
#include <QDialogButtonBox>

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
    auto* layout = new QFormLayout(this);

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

    layout->addRow("Sorting Strategy:", m_sortStrategyCombo);
    layout->addRow("Update Interval:", m_updateIntervalSpin);
    layout->addRow("Grid Rows:", m_gridRowsSpin);
    layout->addRow("Grid Columns:", m_gridColsSpin);
    layout->addRow("Show Best Network Warning:", m_showBestNetworkWarningCheck);

    auto* buttons = new QDialogButtonBox(
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
