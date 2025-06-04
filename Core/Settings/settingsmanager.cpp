#include "settingsmanager.h"

SettingsManager::SettingsManager(QObject* parent)
    : QObject(parent),
    m_settings("Pobeda4isto", "NetworkDashboard")
{

}

QString SettingsManager::getSortStrategy() const
{
    return readSetting("SortStrategy", "Speed").toString();
}

int SettingsManager::getUpdateInterval() const
{
    return readSetting("UpdateInterval", 2000).toInt();
}

int SettingsManager::getGridRows() const
{
    return readSetting("GridRows", 3).toInt();
}

int SettingsManager::getGridCols() const
{
    return readSetting("GridCols", 3).toInt();
}

bool SettingsManager::getShowBestNetworkWarning() const
{
    return readSetting("ShowBestNetworkWarning", true).toBool();
}

QString SettingsManager::getDataUnits() const
{
    return readSetting("DataUnits", "MB").toString();
}

int SettingsManager::getDecimalPrecision() const
{
    return readSetting("DecimalPrecision", 2).toInt();
}

bool SettingsManager::getAutoRefresh() const
{
    return readSetting("AutoRefresh", true).toBool();
}

QString SettingsManager::getBestNetworkCriteria() const
{
    return readSetting("BestNetworkCriteria", "Speed").toString();
}

QString SettingsManager::getGridUpdateStrategy() const
{
    return readSetting("GridUpdateStrategy", "FullUpdate").toString();
}

void SettingsManager::setSortStrategy(const QString& strategy)
{
    writeSetting("SortStrategy", strategy);
    emit sortStrategyChanged(strategy);
    emit settingsChanged();
}

void SettingsManager::setUpdateInterval(int interval)
{
    writeSetting("UpdateInterval", interval);
    emit updateIntervalChanged(interval);
    emit settingsChanged();
}

void SettingsManager::setGridRows(int rows)
{
    writeSetting("GridRows", rows);
    emit gridRowsChanged(rows);
    emit settingsChanged();
}

void SettingsManager::setGridCols(int cols)
{
    writeSetting("GridCols", cols);
    emit gridColsChanged(cols);
    emit settingsChanged();
}

void SettingsManager::setShowBestNetworkWarning(bool show)
{
    writeSetting("ShowBestNetworkWarning", show);
    emit showBestNetworkWarningChanged(show);
    emit settingsChanged();
}

void SettingsManager::setDataUnits(const QString& units)
{
    writeSetting("DataUnits", units);
    emit dataUnitsChanged(units);
    emit settingsChanged();
}

void SettingsManager::setDecimalPrecision(int precision)
{
    writeSetting("DecimalPrecision", precision);
    emit decimalPrecisionChanged(precision);
    emit settingsChanged();
}

void SettingsManager::setAutoRefresh(bool enable)
{
    writeSetting("AutoRefresh", enable);
    emit autoRefreshChanged(enable);
    emit settingsChanged();
}

void SettingsManager::setBestNetworkCriteria(const QString& criteria)
{
    writeSetting("BestNetworkCriteria", criteria);
    emit bestNetworkCriteriaChanged(criteria);
    emit settingsChanged();
}

void SettingsManager::setGridUpdateStrategy(const QString &strategy)
{
    writeSetting("GridUpdateStrategy", strategy);
    emit gridUpdateStrategyChanged(strategy);
    emit settingsChanged();
}

QVariant SettingsManager::readSetting(const QString& key, const QVariant& defaultValue) const
{
    //m_settings.beginGroup("Settings");
    QVariant value = m_settings.value(key, defaultValue);
    //m_settings.endGroup();
    return value;
}

void SettingsManager::writeSetting(const QString& key, const QVariant& value)
{
   // m_settings.beginGroup("Settings");
    m_settings.setValue(key, value);
    //m_settings.endGroup();
}
