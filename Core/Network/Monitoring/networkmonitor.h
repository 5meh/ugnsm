#ifndef NETWORKMONITOR_H
#define NETWORKMONITOR_H

#include <QObject>
#include <QHash>
#include <QTimer>

class TaskScheduler;

class NetworkMonitor: public QObject
{
    Q_OBJECT
public:
    explicit NetworkMonitor(TaskScheduler* scheduler = nullptr, QObject* parent = nullptr);

    void startMonitoring(int intervalMs = 1000);
    void stopMonitoring();

signals:
    void statsUpdated(const QString& mac,
                      quint64 downloadSpeedBps,
                      quint64 uploadSpeedBps);
    // public slots:
    //     void onNetworkStatsUpdated(const QString& interface, quint64 rx, quint64 tx);

private slots:
    void refreshStats();
    void monitoringLoop();

private:
    struct InterfaceStats
    {
        quint64 rxBytes = 0;
        quint64 txBytes = 0;
        qint64 lastUpdate = 0;
    };

    bool getInterfaceStats(QHash<QString, InterfaceStats>& currentStats);
    void calculateSpeeds(const QHash<QString, InterfaceStats>& currentStats);

    // Platform-specific implementation
    bool readRawInterfaceStats(QHash<QString, InterfaceStats>& stats);

    TaskScheduler* m_scheduler;
    QAtomicInt m_running{0};
    int m_interval;

    QHash<QString, InterfaceStats> m_previousStats;
};

#endif // NETWORKMONITOR_H
