#ifndef NETWORKMONITOR_H
#define NETWORKMONITOR_H

#include <QObject>
#include <QHash>
#include <QSet>
#include <QMutex>
#include <QAtomicInt>
#include <QDateTime>

class NetworkMonitor : public QObject
{
    Q_OBJECT
public:
    struct InterfaceStats {
        quint64 rxBytes = 0;
        quint64 txBytes = 0;
        qint64 lastUpdate = 0;
        bool initialized = false;
    };

    explicit NetworkMonitor(QObject *parent = nullptr);
    ~NetworkMonitor() = default;

    static QString normalizeMac(const QString& raw);

    void startMonitoring(int intervalMs = 1000);
    void stopMonitoring();
    void initializeStats(const QSet<QString>& macs);
    void updateTrackedMacs(const QSet<QString>& macs);

signals:
    void statsUpdated(QString mac, quint64 rxSpeed, quint64 txSpeed);

private slots:
    void refreshStats();

private:
    bool readRawInterfaceStats(QHash<QString, InterfaceStats>& stats);

    QAtomicInt m_running;
    int m_interval;
    QMutex m_interfaceMutex;
    bool m_initialized;

    QSet<QString> m_trackedMacs;
    QHash<QString, InterfaceStats> m_previousStats;
};

#endif // NETWORKMONITOR_H
