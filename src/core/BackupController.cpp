#include "BackupController.h"
#include <QDateTime>
#include <QDir>
#include <QDebug>

BackupController::BackupController(HistoryManager *historyManager, QObject *parent)
    : QObject(parent), m_historyManager(historyManager)
{
    connect(&m_rsyncProcess, &RsyncProcess::stateChanged, this, &BackupController::stateChanged);
    connect(&m_rsyncProcess, &RsyncProcess::progressUpdated, this, &BackupController::progressUpdated);
    connect(&m_rsyncProcess, &RsyncProcess::logOutput, this, &BackupController::logMessage);
    connect(&m_rsyncProcess, &RsyncProcess::finished, this, &BackupController::onRsyncFinished);

    connect(&m_systemMonitor, &SystemMonitor::metricsUpdated, this, &BackupController::onMetricsUpdated);
    connect(&m_systemMonitor, &SystemMonitor::loadStateChanged, this, &BackupController::onLoadStateChanged);
}

void BackupController::startBackup(const BackupProfile &profile)
{
    m_currentProfile = profile;
    m_autoPaused = false;
    m_startDateTime = QDateTime::currentDateTime();

    // Set monitor thresholds according to profile settings
    m_systemMonitor.setCpuThreshold(profile.cpuThreshold);
    m_systemMonitor.setLoadAvgThreshold(profile.loadAvgThreshold);
    m_systemMonitor.startMonitoring(2000);

    // Ensure target directory exists
    QDir().mkpath(profile.targetDir);

    emit logMessage(tr("=== Backup Started: %1 ===").arg(profile.name));
    emit logMessage(tr("Start Time: %1").arg(m_startDateTime.toString("yyyy-MM-dd HH:mm:ss")));
    emit logMessage(tr("Source: %1").arg(profile.sourceDir));
    emit logMessage(tr("Target: %1").arg(profile.targetDir));

    // Pre-check: if system load is already high
    if (profile.autoPauseOnHighLoad && m_systemMonitor.isHighLoad()) {
        emit loadWarning(tr("System load is high. Waiting for load to decrease before starting..."));
        emit logMessage(tr("[WARN] Backup start paused due to high load..."));
    }

    // Perform direct synchronization
    m_rsyncProcess.startBackup(profile.sourceDir, profile.targetDir, QString(), profile.excludePatterns, true);
}

void BackupController::pauseBackup()
{
    m_rsyncProcess.pauseBackup();
    emit logMessage(tr("[INFO] Manually paused"));
}

void BackupController::resumeBackup()
{
    m_autoPaused = false;
    m_rsyncProcess.resumeBackup();
    emit logMessage(tr("[INFO] Backup resumed"));
}

void BackupController::cancelBackup()
{
    m_rsyncProcess.cancelBackup();
    m_systemMonitor.stopMonitoring();
    emit logMessage(tr("[WARN] Backup canceled"));
}

void BackupController::onMetricsUpdated(double cpuPercent, double loadAvg)
{
    Q_UNUSED(cpuPercent);
    Q_UNUSED(loadAvg);
}

void BackupController::onLoadStateChanged(bool isHighLoad)
{
    if (!m_currentProfile.autoPauseOnHighLoad) return;

    if (isHighLoad && m_rsyncProcess.state() == RsyncProcess::Running) {
        m_autoPaused = true;
        m_rsyncProcess.pauseBackup();
        QString warn = tr("High load detected (CPU: %1%, Load: %2) - Backup auto-paused")
                       .arg(m_systemMonitor.getCpuUsagePercent(), 0, 'f', 1)
                       .arg(m_systemMonitor.getLoadAverage1Min(), 0, 'f', 2);
        emit loadWarning(warn);
        emit logMessage("[PAUSE] " + warn);
    } else if (!isHighLoad && m_autoPaused && m_rsyncProcess.state() == RsyncProcess::Paused) {
        m_autoPaused = false;
        m_rsyncProcess.resumeBackup();
        QString info = tr("System load decreased - Backup resumed");
        emit logMessage("[RESUME] " + info);
    }
}

void BackupController::onRsyncFinished(int exitCode, bool success)
{
    m_systemMonitor.stopMonitoring();
    QDateTime endDateTime = QDateTime::currentDateTime();

    // Record execution history
    if (m_historyManager) {
        HistoryRecord rec;
        rec.profileId = m_currentProfile.id;
        rec.profileName = m_currentProfile.name;
        rec.startDateTime = m_startDateTime.toString("yyyy-MM-dd HH:mm:ss");
        rec.endDateTime = endDateTime.toString("yyyy-MM-dd HH:mm:ss");
        rec.success = success;
        rec.detailMessage = success ? tr("Completed successfully") : m_rsyncProcess.lastError();
        m_historyManager->addRecord(rec);
    }

    if (success) {
        emit logMessage(tr("=== Backup Completed Successfully (Exit Code: %1) ===").arg(exitCode));
        emit logMessage(tr("End Time: %1").arg(endDateTime.toString("yyyy-MM-dd HH:mm:ss")));
        emit backupFinished(true, tr("Backup completed successfully."));
    } else {
        QString err = m_rsyncProcess.lastError();
        emit logMessage(tr("=== Backup Failed: %1 ===").arg(err));
        emit backupFinished(false, tr("Backup error: %1").arg(err));
    }
}

QString BackupController::findLatestSnapshotDir(const QString &baseTargetDir)
{
    Q_UNUSED(baseTargetDir);
    return QString();
}
