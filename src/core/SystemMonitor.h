#ifndef SYSTEMMONITOR_H
#define SYSTEMMONITOR_H

#include <QObject>
#include <QTimer>

class SystemMonitor : public QObject
{
    Q_OBJECT
public:
    explicit SystemMonitor(QObject *parent = nullptr);
    ~SystemMonitor() override = default;

    void startMonitoring(int intervalMs = 3000);
    void stopMonitoring();

    double getLoadAverage1Min() const;
    double getCpuUsagePercent() const;

    void setCpuThreshold(double percent) { m_cpuThreshold = percent; }
    void setLoadAvgThreshold(double load) { m_loadAvgThreshold = load; }
    
    double cpuThreshold() const { return m_cpuThreshold; }
    double loadAvgThreshold() const { return m_loadAvgThreshold; }

    bool isHighLoad() const;

signals:
    void metricsUpdated(double cpuPercent, double loadAvg);
    void loadStateChanged(bool isHighLoad);

private slots:
    void checkMetrics();

private:
    QTimer m_timer;
    double m_cpuThreshold = 80.0;     // デフォルト 80%
    double m_loadAvgThreshold = 4.0;  // デフォルト 4.0
    bool m_wasHighLoad = false;

    // CPU使用率パース用の前回の値
    unsigned long long m_prevIdleTicks = 0;
    unsigned long long m_prevTotalTicks = 0;

    double readLoadAverage();
    double readCpuUsage();
};

#endif // SYSTEMMONITOR_H
