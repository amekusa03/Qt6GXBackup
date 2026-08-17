#include "HistoryManager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDir>
#include <QStandardPaths>
#include <QUuid>

QJsonObject HistoryRecord::toJson() const
{
    QJsonObject json;
    json["id"] = id;
    json["profileId"] = profileId;
    json["profileName"] = profileName;
    json["startDateTime"] = startDateTime;
    json["endDateTime"] = endDateTime;
    json["success"] = success;
    json["detailMessage"] = detailMessage;
    return json;
}

HistoryRecord HistoryRecord::fromJson(const QJsonObject &json)
{
    HistoryRecord r;
    r.id = json["id"].toString();
    r.profileId = json["profileId"].toString();
    r.profileName = json["profileName"].toString();
    r.startDateTime = json["startDateTime"].toString();
    r.endDateTime = json["endDateTime"].toString();
    r.success = json["success"].toBool(false);
    r.detailMessage = json["detailMessage"].toString();
    return r;
}

HistoryManager::HistoryManager(QObject *parent)
    : QObject(parent)
{
    loadHistory();
}

QString HistoryManager::getConfigFilePath() const
{
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(configDir);
    return QDir(configDir).filePath("history.json");
}

bool HistoryManager::loadHistory()
{
    m_history.clear();
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
            m_history.append(HistoryRecord::fromJson(val.toObject()));
        }
    }

    emit historyChanged();
    return true;
}

bool HistoryManager::saveHistory()
{
    QJsonArray array;
    for (const auto &r : m_history) {
        array.append(r.toJson());
    }

    QJsonDocument doc(array);
    QFile file(getConfigFilePath());
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    file.write(doc.toJson(QJsonDocument::Indented));
    emit historyChanged();
    return true;
}

void HistoryManager::addRecord(const HistoryRecord &record)
{
    HistoryRecord r = record;
    if (r.id.isEmpty()) {
        r.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    }
    // 最新のものが先頭にくるように挿入
    m_history.prepend(r);
    saveHistory();
}

HistoryRecord HistoryManager::getLatestRecordForProfile(const QString &profileId) const
{
    for (const auto &r : m_history) {
        if (r.profileId == profileId && r.success) {
            return r;
        }
    }
    return HistoryRecord();
}
