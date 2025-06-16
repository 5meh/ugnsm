#include "settingsmanager.h"

#include "../Utilities/Logger/logger.h"

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
    if (getSortStrategy() == strategy)
        return;
    writeSetting("SortStrategy", strategy);
    emit sortStrategyChanged(strategy);
}

void SettingsManager::setUpdateInterval(int interval)
{
    if (getUpdateInterval() == interval)
        return;
    writeSetting("UpdateInterval", interval);
    emit updateIntervalChanged(interval);
}

void SettingsManager::setShowBestNetworkWarning(bool show)
{
    if (getShowBestNetworkWarning() == show)
        return;
    writeSetting("ShowBestNetworkWarning", show);
    emit showBestNetworkWarningChanged(show);
}

void SettingsManager::setDataUnits(const QString& units)
{
    if (getDataUnits() == units)
        return;
    writeSetting("DataUnits", units);
    emit dataUnitsChanged(units);
}

void SettingsManager::setDecimalPrecision(int precision)
{
    if (getDecimalPrecision() == precision)
        return;
    writeSetting("DecimalPrecision", precision);
    emit decimalPrecisionChanged(precision);
}

void SettingsManager::setAutoRefresh(bool enable)
{
    if (getAutoRefresh() == enable)
        return;
    writeSetting("AutoRefresh", enable);
    emit autoRefreshChanged(enable);
}

void SettingsManager::setBestNetworkCriteria(const QString& criteria)
{
    if (getBestNetworkCriteria() == criteria)
        return;
    writeSetting("BestNetworkCriteria", criteria);
    emit bestNetworkCriteriaChanged(criteria);
}

void SettingsManager::setGridUpdateStrategy(const QString& strategy)
{
    if (getGridUpdateStrategy() == strategy)
        return;
    writeSetting("GridUpdateStrategy", strategy);
    emit gridUpdateStrategyChanged(strategy);
}

QVariant SettingsManager::readSetting(const QString& key, const QVariant& defaultValue) const
{
    QVariant value = m_settings.value(key, defaultValue);
    return value;
}

void SettingsManager::writeSetting(const QString& key, const QVariant& value)
{
    m_settings.setValue(key, value);
    Logger::instance().log(Logger::Info,
                           QString("Setting changed: %1 = %2").arg(key).arg(value.toString()),
                           "Settings");
}
