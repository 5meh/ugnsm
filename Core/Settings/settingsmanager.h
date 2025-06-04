#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QObject>
#include <QSettings>
#include <QMutex>

class SettingsManager : public QObject
{
    Q_OBJECT

    //TODO:mb remove Q_PROPERTY
    Q_PROPERTY(QString gridUpdateStrategy READ getGridUpdateStrategy WRITE setGridUpdateStrategy NOTIFY gridUpdateStrategyChanged)
    Q_PROPERTY(QString sortStrategy READ getSortStrategy WRITE setSortStrategy NOTIFY sortStrategyChanged)
    Q_PROPERTY(int updateInterval READ getUpdateInterval WRITE setUpdateInterval NOTIFY updateIntervalChanged)
    Q_PROPERTY(int gridRows READ getGridRows WRITE setGridRows NOTIFY gridRowsChanged)
    Q_PROPERTY(int gridCols READ getGridCols WRITE setGridCols NOTIFY gridColsChanged)
    Q_PROPERTY(bool showBestNetworkWarning READ getShowBestNetworkWarning WRITE setShowBestNetworkWarning NOTIFY showBestNetworkWarningChanged)
    Q_PROPERTY(QString dataUnits READ getDataUnits WRITE setDataUnits NOTIFY dataUnitsChanged)
    Q_PROPERTY(int decimalPrecision READ getDecimalPrecision WRITE setDecimalPrecision NOTIFY decimalPrecisionChanged)
    Q_PROPERTY(bool autoRefresh READ getAutoRefresh WRITE setAutoRefresh NOTIFY autoRefreshChanged)
    Q_PROPERTY(QString bestNetworkCriteria READ getBestNetworkCriteria WRITE setBestNetworkCriteria NOTIFY bestNetworkCriteriaChanged)    

public:
    explicit SettingsManager(QObject* parent = nullptr);

    QString getSortStrategy() const;
    int getUpdateInterval() const;
    int getGridRows() const;
    int getGridCols() const;
    bool getShowBestNetworkWarning() const;
    QString getDataUnits() const;
    int getDecimalPrecision() const;
    bool getAutoRefresh() const;
    QString getBestNetworkCriteria() const;
    QString getGridUpdateStrategy() const;

    void setSortStrategy(const QString& strategy);
    void setUpdateInterval(int interval);
    void setGridRows(int rows);
    void setGridCols(int cols);
    void setShowBestNetworkWarning(bool show);
    void setDataUnits(const QString& units);
    void setDecimalPrecision(int precision);
    void setAutoRefresh(bool enable);
    void setBestNetworkCriteria(const QString& criteria);
    void setGridUpdateStrategy(const QString& strategy);

signals:
    void settingsChanged();
    void sortStrategyChanged(const QString& strategy);
    void updateIntervalChanged(int interval);
    void gridRowsChanged(int rows);
    void gridColsChanged(int cols);
    void showBestNetworkWarningChanged(bool show);
    void dataUnitsChanged(const QString& units);
    void decimalPrecisionChanged(int precision);
    void autoRefreshChanged(bool enable);
    void bestNetworkCriteriaChanged(const QString& criteria);
    void gridUpdateStrategyChanged(const QString& strategy);

private:

    QVariant readSetting(const QString& key, const QVariant& defaultValue = QVariant()) const;
    void writeSetting(const QString& key, const QVariant& value);

    mutable QSettings m_settings;
};

#endif // SETTINGSMANAGER_H
