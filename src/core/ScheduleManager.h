#ifndef SCHEDULEMANAGER_H
#define SCHEDULEMANAGER_H

#include <QObject>
#include <QTimer>
#include <QDateTime>
#include "ProfileManager.h"
#include "BackupController.h"
#include "HistoryManager.h"

class ScheduleManager : public QObject
{
    Q_OBJECT
public:
    explicit ScheduleManager(ProfileManager *profileManager,
                            BackupController *backupController,
                            HistoryManager *historyManager,
                            QObject *parent = nullptr);
    ~ScheduleManager() override = default;

    void start();
    void stop();

    // Function to check if a schedule was missed (catch-up detection)
    bool isScheduleMissed(const BackupProfile &profile, const QDateTime &lastEndTime, const QDateTime &now) const;

signals:
    void scheduledBackupTriggered(const BackupProfile &profile, bool isCatchUp);
    void scheduleDelayed(const QString &profileName, const QString &reason);

private slots:
    void checkSchedules();

private:
    ProfileManager *m_profileManager;
    BackupController *m_backupController;
    HistoryManager *m_historyManager;
    QTimer m_timer;
};

#endif // SCHEDULEMANAGER_H
