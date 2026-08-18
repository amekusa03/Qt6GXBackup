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
    m_timer.start(60000); // Check every minute
    checkSchedules();
}

void ScheduleManager::stop()
{
    m_timer.stop();
}

bool ScheduleManager::isScheduleMissed(const BackupProfile &profile, const QDateTime &lastEndTime, const QDateTime &now) const
{
    if (!profile.scheduleEnabled) return false;

    // If there is no previous backup record, consider it missed and execute
    if (!lastEndTime.isValid()) {
        return true;
    }

    QTime schedTime = QTime::fromString(profile.scheduleTime, "HH:mm");
    if (!schedTime.isValid()) return false;

    // Check all dates between lastEndTime.date() and now.date()
    QDate curDate = lastEndTime.date();
    QDate endDate = now.date();

    while (curDate <= endDate) {
        QDateTime candidateSlot(curDate, schedTime);
        
        // Check if candidate slot is within (lastEndTime < candidateSlot <= now)
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

    // Skip if backup is currently running or paused
    if (m_backupController->state() == RsyncProcess::Running || m_backupController->state() == RsyncProcess::Paused) {
        return;
    }

    QDateTime now = QDateTime::currentDateTime();
    auto profiles = m_profileManager->profiles();

    for (auto &profile : profiles) {
        if (!profile.scheduleEnabled) continue;

        // Retrieve latest successful backup record
        HistoryRecord latestRec = m_historyManager->getLatestRecordForProfile(profile.id);
        QDateTime lastEndTime = QDateTime::fromString(latestRec.endDateTime, "yyyy-MM-dd HH:mm:ss");

        // Determine if scheduled time was missed
        if (isScheduleMissed(profile, lastEndTime, now)) {
            // Check system load
            if (profile.autoPauseOnHighLoad && m_backupController->monitor()->isHighLoad()) {
                emit scheduleDelayed(profile.name, tr("Auto backup postponed due to high load..."));
                qDebug() << "Schedule delayed for" << profile.name << "due to high load.";
                continue;
            }

            bool isCatchUp = lastEndTime.isValid() && (lastEndTime.date() < now.date());
            qDebug() << "Triggering scheduled backup (catchUp=" << isCatchUp << ") for:" << profile.name;

            emit scheduledBackupTriggered(profile, isCatchUp);
            m_backupController->startBackup(profile);
            break; // Start only one profile per check cycle
        }
    }
}
