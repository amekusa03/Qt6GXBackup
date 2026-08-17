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

    // モニターの閾値をプロファイル設定に合わせる
    m_systemMonitor.setCpuThreshold(profile.cpuThreshold);
    m_systemMonitor.setLoadAvgThreshold(profile.loadAvgThreshold);
    m_systemMonitor.startMonitoring(2000);

    // 単一フォルダへの直接同期
    QDir().mkpath(profile.targetDir);

    emit logMessage(QString("=== バックアップ開始: %1 ===").arg(profile.name));
    emit logMessage(QString("開始日時: %1").arg(m_startDateTime.toString("yyyy-MM-dd HH:mm:ss")));
    emit logMessage(QString("ソース: %1").arg(profile.sourceDir));
    emit logMessage(QString("ターゲット (単一ミラーフォルダ): %1").arg(profile.targetDir));

    // 事前チェック：すでに高負荷の場合
    if (profile.autoPauseOnHighLoad && m_systemMonitor.isHighLoad()) {
        emit loadWarning("システムが高負荷です。負荷の低下を待って開始します...");
        emit logMessage("[WARN] 高負荷のためバックアップ開始を待機中...");
    }

    // タイムスタンプサブフォルダや --link-dest は使用せず直接同期
    m_rsyncProcess.startBackup(profile.sourceDir, profile.targetDir, QString(), profile.excludePatterns, true);
}

void BackupController::pauseBackup()
{
    m_rsyncProcess.pauseBackup();
    emit logMessage("[INFO] 手動で一時停止しました");
}

void BackupController::resumeBackup()
{
    m_autoPaused = false;
    m_rsyncProcess.resumeBackup();
    emit logMessage("[INFO] バックアップを再開しました");
}

void BackupController::cancelBackup()
{
    m_rsyncProcess.cancelBackup();
    m_systemMonitor.stopMonitoring();
    emit logMessage("[WARN] バックアップがキャンセルされました");
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
        QString warn = QString("高負荷を検知 (CPU: %1%, Load: %2) - バックアップを自動一時停止")
                       .arg(m_systemMonitor.getCpuUsagePercent(), 0, 'f', 1)
                       .arg(m_systemMonitor.getLoadAverage1Min(), 0, 'f', 2);
        emit loadWarning(warn);
        emit logMessage("[PAUSE] " + warn);
    } else if (!isHighLoad && m_autoPaused && m_rsyncProcess.state() == RsyncProcess::Paused) {
        m_autoPaused = false;
        m_rsyncProcess.resumeBackup();
        QString info = "システム負荷が低下したため、バックアップを再開しました";
        emit logMessage("[RESUME] " + info);
    }
}

void BackupController::onRsyncFinished(int exitCode, bool success)
{
    m_systemMonitor.stopMonitoring();
    QDateTime endDateTime = QDateTime::currentDateTime();

    // 履歴の記録
    if (m_historyManager) {
        HistoryRecord rec;
        rec.profileId = m_currentProfile.id;
        rec.profileName = m_currentProfile.name;
        rec.startDateTime = m_startDateTime.toString("yyyy-MM-dd HH:mm:ss");
        rec.endDateTime = endDateTime.toString("yyyy-MM-dd HH:mm:ss");
        rec.success = success;
        rec.detailMessage = success ? "正常完了" : m_rsyncProcess.lastError();
        m_historyManager->addRecord(rec);
    }

    if (success) {
        emit logMessage(QString("=== バックアップ正常完了 (Exit Code: %1) ===").arg(exitCode));
        emit logMessage(QString("終了日時: %1").arg(endDateTime.toString("yyyy-MM-dd HH:mm:ss")));
        emit backupFinished(true, "バックアップが正常に完了しました。");
    } else {
        QString err = m_rsyncProcess.lastError();
        emit logMessage(QString("=== バックアップ失敗: %1 ===").arg(err));
        emit backupFinished(false, QString("バックアップエラー: %1").arg(err));
    }
}

QString BackupController::findLatestSnapshotDir(const QString &baseTargetDir)
{
    Q_UNUSED(baseTargetDir);
    return QString();
}
