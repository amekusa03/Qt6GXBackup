#ifndef HISTORYMANAGER_H
#define HISTORYMANAGER_H

#include <QObject>
#include <QString>
#include <QList>
#include <QJsonObject>
#include <QDateTime>

struct HistoryRecord {
    QString id;
    QString profileId;
    QString profileName;
    QString startDateTime; // yyyy-MM-dd HH:mm:ss
    QString endDateTime;   // yyyy-MM-dd HH:mm:ss
    bool success = false;
    QString detailMessage;

    QJsonObject toJson() const;
    static HistoryRecord fromJson(const QJsonObject &json);
};

class HistoryManager : public QObject
{
    Q_OBJECT
public:
    explicit HistoryManager(QObject *parent = nullptr);
    ~HistoryManager() override = default;

    bool loadHistory();
    bool saveHistory();

    QList<HistoryRecord> history() const { return m_history; }
    void addRecord(const HistoryRecord &record);
    HistoryRecord getLatestRecordForProfile(const QString &profileId) const;

signals:
    void historyChanged();

private:
    QList<HistoryRecord> m_history;
    QString getConfigFilePath() const;
};

#endif // HISTORYMANAGER_H
