#include "ScheduleManager.h"
#include <QTime>
#include <QDate>
#include <QDebug>

ScheduleManager::ScheduleManager(ProfileManager *profileManager,
                                 BackupController *backupController,
                                 HistoryManager *historyManager,
                                 QObject *parent)
    : QObject(parent),
      m_profileManager(profileManager),
      m_backupController(backupController),
      m_historyManager(historyManager)
{
    connect(&m_timer, &QTimer::timeout, this, &ScheduleManager::checkSchedules);
}

void ScheduleManager::start()
{
    m_timer.start(60000); // 1分ごとにチェック
    checkSchedules();
}

void ScheduleManager::stop()
{
    m_timer.stop();
}

bool ScheduleManager::isScheduleMissed(const BackupProfile &profile, const QDateTime &lastEndTime, const QDateTime &now) const
{
    if (!profile.scheduleEnabled) return false;

    // 前回のバックアップ完了履歴がない場合は未実行とみなし即実行が必要
    if (!lastEndTime.isValid()) {
        return true;
    }

    QTime schedTime = QTime::fromString(profile.scheduleTime, "HH:mm");
    if (!schedTime.isValid()) return false;

    // lastEndTime.date() から now.date() までの全日付について調べる
    QDate curDate = lastEndTime.date();
    QDate endDate = now.date();

    while (curDate <= endDate) {
        QDateTime candidateSlot(curDate, schedTime);
        
        // 候補の予定日時 candidateSlot が (lastEndTime < candidateSlot <= now) の範囲にあるか
        if (candidateSlot > lastEndTime && candidateSlot <= now) {
            if (profile.scheduleType == "daily") {
                return true;
            } else if (profile.scheduleType == "weekly") {
                int dayOfWeek = curDate.dayOfWeek(); // 1=Mon ... 7=Sun
                if (profile.scheduleDays.contains(dayOfWeek)) {
                    return true;
                }
            }
        }
        curDate = curDate.addDays(1);
    }

    return false;
}

void ScheduleManager::checkSchedules()
{
    if (!m_profileManager || !m_backupController || !m_historyManager) return;

    // 現在バックアップ実行中の場合はスキップ
    if (m_backupController->state() == RsyncProcess::Running || m_backupController->state() == RsyncProcess::Paused) {
        return;
    }

    QDateTime now = QDateTime::currentDateTime();
    auto profiles = m_profileManager->profiles();

    for (auto &profile : profiles) {
        if (!profile.scheduleEnabled) continue;

        // 前回のバックアップ成功レコードを取得
        HistoryRecord latestRec = m_historyManager->getLatestRecordForProfile(profile.id);
        QDateTime lastEndTime = QDateTime::fromString(latestRec.endDateTime, "yyyy-MM-dd HH:mm:ss");

        // 前回終了日時がスケジュール予定を過ぎているか判定
        if (isScheduleMissed(profile, lastEndTime, now)) {
            // 高負荷チェック
            if (profile.autoPauseOnHighLoad && m_backupController->monitor()->isHighLoad()) {
                emit scheduleDelayed(profile.name, "高負荷のため自動バックアップの実行を一時保留中...");
                qDebug() << "Schedule delayed for" << profile.name << "due to high load.";
                continue;
            }

            bool isCatchUp = lastEndTime.isValid() && (lastEndTime.date() < now.date());
            qDebug() << "Triggering scheduled backup (catchUp=" << isCatchUp << ") for:" << profile.name;

            emit scheduledBackupTriggered(profile, isCatchUp);
            m_backupController->startBackup(profile);
            break; // 1回につき1プロファイルのみ起動
        }
    }
}
