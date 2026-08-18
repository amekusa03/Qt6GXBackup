#include "ProfileManager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDir>
#include <QStandardPaths>
#include <QUuid>
#include <QCoreApplication>
#include <QDebug>

QJsonObject BackupProfile::toJson() const
{
    QJsonObject json;
    json["id"] = id;
    json["name"] = name;
    json["sourceDir"] = sourceDir;
    json["targetDir"] = targetDir;
    json["excludePatterns"] = QJsonArray::fromStringList(excludePatterns);
    json["scheduleEnabled"] = scheduleEnabled;
    json["scheduleType"] = scheduleType;
    json["scheduleTime"] = scheduleTime;

    QJsonArray daysArray;
    for (int day : scheduleDays) {
        daysArray.append(day);
    }
    json["scheduleDays"] = daysArray;
    json["lastRunDateTime"] = lastRunDateTime;

    json["autoPauseOnHighLoad"] = autoPauseOnHighLoad;
    json["cpuThreshold"] = cpuThreshold;
    json["loadAvgThreshold"] = loadAvgThreshold;
    return json;
}

BackupProfile BackupProfile::fromJson(const QJsonObject &json)
{
    BackupProfile p;
    p.id = json["id"].toString();
    p.name = json["name"].toString();
    p.sourceDir = json["sourceDir"].toString();
    p.targetDir = json["targetDir"].toString();

    QJsonArray exArray = json["excludePatterns"].toArray();
    for (const auto &val : exArray) {
        p.excludePatterns.append(val.toString());
    }

    p.scheduleEnabled = json["scheduleEnabled"].toBool(false);
    p.scheduleType = json["scheduleType"].toString("daily");
    p.scheduleTime = json["scheduleTime"].toString("02:00");

    QJsonArray daysArray = json["scheduleDays"].toArray();
    for (const auto &val : daysArray) {
        p.scheduleDays.append(val.toInt());
    }
    p.lastRunDateTime = json["lastRunDateTime"].toString();

    p.autoPauseOnHighLoad = json["autoPauseOnHighLoad"].toBool(true);
    p.cpuThreshold = json["cpuThreshold"].toDouble(80.0);
    p.loadAvgThreshold = json["loadAvgThreshold"].toDouble(4.0);

    return p;
}

ProfileManager::ProfileManager(QObject *parent)
    : QObject(parent)
{
    loadProfiles();
}

QString ProfileManager::getConfigFilePath() const
{
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(configDir);
    return QDir(configDir).filePath("profiles.json");
}

bool ProfileManager::loadProfiles()
{
    m_profiles.clear();
    QFile file(getConfigFilePath());
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isArray()) return false;

    QJsonArray array = doc.array();
    for (const auto &val : array) {
        if (val.isObject()) {
            m_profiles.append(BackupProfile::fromJson(val.toObject()));
        }
    }

    emit profilesChanged();
    return true;
}

bool ProfileManager::saveProfiles()
{
    QJsonArray array;
    for (const auto &p : m_profiles) {
        array.append(p.toJson());
    }

    QJsonDocument doc(array);
    QFile file(getConfigFilePath());
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    file.write(doc.toJson(QJsonDocument::Indented));
    emit profilesChanged();
    return true;
}

BackupProfile ProfileManager::profile(const QString &id) const
{
    for (const auto &p : m_profiles) {
        if (p.id == id) return p;
    }
    return BackupProfile();
}

void ProfileManager::addProfile(const BackupProfile &profile)
{
    BackupProfile p = profile;
    if (p.id.isEmpty()) {
        p.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    }
    m_profiles.append(p);
    saveProfiles();
}

void ProfileManager::updateProfile(const BackupProfile &profile)
{
    for (int i = 0; i < m_profiles.size(); ++i) {
        if (m_profiles[i].id == profile.id) {
            m_profiles[i] = profile;
            saveProfiles();
            return;
        }
    }
}

void ProfileManager::removeProfile(const QString &id)
{
    for (int i = 0; i < m_profiles.size(); ++i) {
        if (m_profiles[i].id == id) {
            m_profiles.removeAt(i);
            saveProfiles();
            return;
        }
    }
}

BackupProfile ProfileManager::createUserDataPreset(const QString &targetDir)
{
    BackupProfile p;
    p.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    p.name = QCoreApplication::translate("ProfileManager", "User Data Backup");
    p.sourceDir = QDir::homePath();
    p.targetDir = targetDir;
    p.excludePatterns = QStringList{
        ".cache",
        "Downloads",
        ".local/share/Trash",
        ".thumbnails",
        "*.tmp"
    };
    p.scheduleEnabled = true;
    p.scheduleType = "daily";
    p.scheduleTime = "03:00";
    p.autoPauseOnHighLoad = true;
    p.cpuThreshold = 80.0;
    p.loadAvgThreshold = 4.0;
    return p;
}

BackupProfile ProfileManager::createSystemPreset(const QString &targetDir)
{
    BackupProfile p;
    p.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    p.name = QCoreApplication::translate("ProfileManager", "Full System Backup");
    p.sourceDir = "/";
    p.targetDir = targetDir;
    p.excludePatterns = QStringList{
        "/proc/*",
        "/sys/*",
        "/dev/*",
        "/run/*",
        "/mnt/*",
        "/media/*",
        "/tmp/*",
        "/lost+found",
        "/var/tmp/*",
        "/var/cache/*",
        "/home/*/.cache/*"
    };
    p.scheduleEnabled = true;
    p.scheduleType = "weekly";
    p.scheduleTime = "04:00";
    p.scheduleDays = QList<int>{7}; // Sunday
    p.autoPauseOnHighLoad = true;
    p.cpuThreshold = 75.0;
    p.loadAvgThreshold = 3.5;
    return p;
}
