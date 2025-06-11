#ifndef NETWORKMONITOR_H
#define NETWORKMONITOR_H

#include <QObject>
#include <QHash>
#include <QTimer>
#include <QMutex>
#include <QSet>

class NetworkMonitor: public QObject
{
    Q_OBJECT
public:
    explicit NetworkMonitor(QObject* parent = nullptr);

    void startMonitoring(int intervalMs = 1000);
    void stopMonitoring();
    void initializeStats(const QSet<QString>& macs);
    void updateTrackedMacs(const QSet<QString>& macs);

signals:
    void statsUpdated(const QString& mac,
                      quint64 downloadSpeedBps,
                      quint64 uploadSpeedBps);

private slots:
    void refreshStats();

private:
    struct InterfaceStats
    {
        quint64 rxBytes = 0;
        quint64 txBytes = 0;
        qint64 lastUpdate = 0;
    };

    //bool getInterfaceStats(QHash<QString, InterfaceStats>& currentStats);
    void calculateSpeeds(const QHash<QString, InterfaceStats>& currentStats);

    // Platform-specific implementation
    bool readRawInterfaceStats(QHash<QString, InterfaceStats>& stats);

    QAtomicInt m_running{0};
    int m_interval;

    QHash<QString, InterfaceStats> m_previousStats;
    QSet<QString> m_trackedMacs;
    QMutex m_interfaceMutex;
    bool m_initialized = false;
};

#endif // NETWORKMONITOR_H
