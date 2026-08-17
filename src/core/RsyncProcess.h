#ifndef RSYNCPROCESS_H
#define RSYNCPROCESS_H

#include <QObject>
#include <QProcess>
#include <QStringList>

class RsyncProcess : public QObject
{
    Q_OBJECT
public:
    enum State {
        Idle,
        Running,
        Paused,
        Finished,
        Error
    };
    Q_ENUM(State)

    explicit RsyncProcess(QObject *parent = nullptr);
    ~RsyncProcess() override;

    void startBackup(const QString &sourceDir,
                     const QString &targetDir,
                     const QString &linkDestDir = QString(),
                     const QStringList &excludeList = QStringList(),
                     bool useNice = true);

    void pauseBackup();
    void resumeBackup();
    void cancelBackup();

    State state() const { return m_state; }
    QString lastError() const { return m_lastError; }
    qint64 currentPid() const;

signals:
    void stateChanged(RsyncProcess::State state);
    void progressUpdated(int percent, QString speed, QString transferred, QString remainingTime);
    void logOutput(const QString &line);
    void finished(int exitCode, bool success);

private slots:
    void onReadyReadStandardOutput();
    void onReadyReadStandardError();
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    QProcess m_process;
    State m_state = Idle;
    QString m_lastError;
    QString m_buffer;

    bool parseProgressLine(const QString &line);
    void setState(State state);
};

#endif // RSYNCPROCESS_H
