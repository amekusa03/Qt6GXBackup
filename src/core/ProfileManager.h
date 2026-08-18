#ifndef PROFILEMANAGER_H
#define PROFILEMANAGER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QList>
#include <QJsonObject>

struct BackupProfile {
    QString id;
    QString name;
    QString sourceDir;
    QString targetDir;
    QStringList excludePatterns;
    
    // Schedule settings
    bool scheduleEnabled = false;
    QString scheduleType = "daily"; // "daily", "weekly"
    QString scheduleTime = "02:00"; // HH:mm
    QList<int> scheduleDays;        // 1=Monday ... 7=Sunday (for weekly schedule)
    QString lastRunDateTime;       // Last execution date & time

    // Smart load throttling settings
    bool autoPauseOnHighLoad = true;
    double cpuThreshold = 80.0;
    double loadAvgThreshold = 4.0;

    QJsonObject toJson() const;
    static BackupProfile fromJson(const QJsonObject &json);
};

class ProfileManager : public QObject
{
    Q_OBJECT
public:
    explicit ProfileManager(QObject *parent = nullptr);
    ~ProfileManager() override = default;

    bool loadProfiles();
    bool saveProfiles();

    QList<BackupProfile> profiles() const { return m_profiles; }
    BackupProfile profile(const QString &id) const;

    void addProfile(const BackupProfile &profile);
    void updateProfile(const BackupProfile &profile);
    void removeProfile(const QString &id);

    // Preset generators
    static BackupProfile createUserDataPreset(const QString &targetDir);
    static BackupProfile createSystemPreset(const QString &targetDir);

signals:
    void profilesChanged();

private:
    QList<BackupProfile> m_profiles;
    QString getConfigFilePath() const;
};

#endif // PROFILEMANAGER_H
