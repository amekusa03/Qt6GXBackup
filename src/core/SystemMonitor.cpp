#include "SystemMonitor.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <fstream>
#include <string>
#include <sstream>

SystemMonitor::SystemMonitor(QObject *parent)
    : QObject(parent)
{
    connect(&m_timer, &QTimer::timeout, this, &SystemMonitor::checkMetrics);
}

void SystemMonitor::startMonitoring(int intervalMs)
{
    m_timer.start(intervalMs);
    checkMetrics();
}

void SystemMonitor::stopMonitoring()
{
    m_timer.stop();
}

double SystemMonitor::readLoadAverage()
{
    QFile file("/proc/loadavg");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        QString line = in.readLine();
        QStringList parts = line.split(QChar(' '), Qt::SkipEmptyParts);
        if (!parts.isEmpty()) {
            bool ok = false;
            double load1 = parts[0].toDouble(&ok);
            if (ok) return load1;
        }
    }
    return 0.0;
}

double SystemMonitor::readCpuUsage()
{
    std::ifstream file("/proc/stat");
    if (!file.is_open()) return 0.0;

    std::string line;
    std::getline(file, line);
    std::istringstream ss(line);

    std::string cpuStr;
    ss >> cpuStr; // "cpu"
    if (cpuStr != "cpu") return 0.0;

    unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;
    if (!(ss >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal)) {
        return 0.0;
    }

    unsigned long long idleTicks = idle + iowait;
    unsigned long long totalTicks = user + nice + system + idle + iowait + irq + softirq + steal;

    unsigned long long totalDiff = totalTicks - m_prevTotalTicks;
    unsigned long long idleDiff = idleTicks - m_prevIdleTicks;

    m_prevTotalTicks = totalTicks;
    m_prevIdleTicks = idleTicks;

    if (totalDiff == 0) return 0.0;

    double cpuPercent = 100.0 * (1.0 - static_cast<double>(idleDiff) / static_cast<double>(totalDiff));
    return cpuPercent < 0.0 ? 0.0 : (cpuPercent > 100.0 ? 100.0 : cpuPercent);
}

void SystemMonitor::checkMetrics()
{
    double loadAvg = readLoadAverage();
    double cpuUsage = readCpuUsage();

    emit metricsUpdated(cpuUsage, loadAvg);

    bool highLoad = (cpuUsage >= m_cpuThreshold) || (loadAvg >= m_loadAvgThreshold);
    if (highLoad != m_wasHighLoad) {
        m_wasHighLoad = highLoad;
        emit loadStateChanged(highLoad);
    }
}

double SystemMonitor::getLoadAverage1Min() const
{
    return const_cast<SystemMonitor*>(this)->readLoadAverage();
}

double SystemMonitor::getCpuUsagePercent() const
{
    return const_cast<SystemMonitor*>(this)->readCpuUsage();
}

bool SystemMonitor::isHighLoad() const
{
    return m_wasHighLoad;
}
