#ifndef BACKUPCONTROLLER_H
#define BACKUPCONTROLLER_H

#include <QObject>
#include <QDateTime>
#include "RsyncProcess.h"
#include "SystemMonitor.h"
#include "ProfileManager.h"
#include "HistoryManager.h"

class BackupController : public QObject
{
    Q_OBJECT
public:
    explicit BackupController(HistoryManager *historyManager, QObject *parent = nullptr);
    ~BackupController() override = default;

    void startBackup(const BackupProfile &profile);
    void pauseBackup();
    void resumeBackup();
    void cancelBackup();

    RsyncProcess::State state() const { return m_rsyncProcess.state(); }
    SystemMonitor* monitor() { return &m_systemMonitor; }
    BackupProfile currentProfile() const { return m_currentProfile; }

signals:
    void stateChanged(RsyncProcess::State state);
    void progressUpdated(int percent, QString speed, QString transferred, QString remainingTime);
    void logMessage(const QString &msg);
    void backupFinished(bool success, const QString &message);
    void loadWarning(const QString &warningMsg);

private slots:
    void onMetricsUpdated(double cpuPercent, double loadAvg);
    void onLoadStateChanged(bool isHighLoad);
    void onRsyncFinished(int exitCode, bool success);

private:
    RsyncProcess m_rsyncProcess;
    SystemMonitor m_systemMonitor;
    HistoryManager *m_historyManager;
    BackupProfile m_currentProfile;
    QDateTime m_startDateTime;
    bool m_autoPaused = false;

    QString generateSnapshotTargetDir(const QString &baseTargetDir);
    QString findLatestSnapshotDir(const QString &baseTargetDir);
};

#endif // BACKUPCONTROLLER_H
