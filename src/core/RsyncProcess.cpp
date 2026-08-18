#include "RsyncProcess.h"
#include <QRegularExpression>
#include <QDebug>
#include <csignal>
#include <unistd.h>

RsyncProcess::RsyncProcess(QObject *parent)
    : QObject(parent)
{
    connect(&m_process, &QProcess::readyReadStandardOutput, this, &RsyncProcess::onReadyReadStandardOutput);
    connect(&m_process, &QProcess::readyReadStandardError, this, &RsyncProcess::onReadyReadStandardError);
    connect(&m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &RsyncProcess::onProcessFinished);
}

RsyncProcess::~RsyncProcess()
{
    m_process.disconnect(this);
    if (m_state == Paused) {
        resumeBackup();
    }
    if (m_process.state() != QProcess::NotRunning) {
        qint64 pid = m_process.processId();
        m_process.terminate();
        if (!m_process.waitForFinished(1000)) {
            if (pid > 0) {
                ::kill(-static_cast<pid_t>(pid), SIGKILL);
            }
            m_process.kill();
            m_process.waitForFinished(500);
        }
    }
}

void RsyncProcess::startBackup(const QString &sourceDir,
                              const QString &targetDir,
                              const QString &linkDestDir,
                              const QStringList &excludeList,
                              bool useNice)
{
    if (m_state == Running || m_state == Paused) {
        qWarning() << "Rsync process already running or paused.";
        return;
    }

    QString program;
    QStringList args;

    if (useNice) {
        program = "nice";
        args << "-n" << "19" << "ionice" << "-c" << "3" << "rsync";
    } else {
        program = "rsync";
    }

    // 基本的なrsyncオプション
    // -a: アーカイブ (パーミッション、所有者、タイムスタンプを維持)
    // -H: ハードリンク保持
    // -A: ACL保持
    // -X: 拡張属性保持
    // --delete: ソースで削除されたファイルをバックアップ先でも削除（ミラーリング）
    // --info=progress2: 進行状況を出力
    args << "-aHAX" << "--delete" << "--info=progress2";

    if (!linkDestDir.isEmpty()) {
        args << QString("--link-dest=%1").arg(linkDestDir);
    }

    for (const QString &exclude : excludeList) {
        if (!exclude.trimmed().isEmpty()) {
            args << QString("--exclude=%1").arg(exclude.trimmed());
        }
    }

    // ソースとターゲット
    QString src = sourceDir;
    if (!src.endsWith('/')) src += '/';
    args << src << targetDir;

    connect(&m_process, &QProcess::readyReadStandardOutput, this, &RsyncProcess::onReadyReadStandardOutput, Qt::UniqueConnection);
    connect(&m_process, &QProcess::readyReadStandardError, this, &RsyncProcess::onReadyReadStandardError, Qt::UniqueConnection);
    connect(&m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &RsyncProcess::onProcessFinished, Qt::UniqueConnection);

    m_lastError.clear();
    m_buffer.clear();
    setState(Running);

    qDebug() << "Executing:" << program << args.join(" ");
    m_process.start(program, args);
}

void RsyncProcess::pauseBackup()
{
    if (m_state != Running) return;

    qint64 pid = m_process.processId();
    if (pid > 0) {
        qDebug() << "Sending SIGSTOP to PID:" << pid;
        ::kill(static_cast<pid_t>(pid), SIGSTOP);
        setState(Paused);
    }
}

void RsyncProcess::resumeBackup()
{
    if (m_state != Paused) return;

    qint64 pid = m_process.processId();
    if (pid > 0) {
        qDebug() << "Sending SIGCONT to PID:" << pid;
        ::kill(static_cast<pid_t>(pid), SIGCONT);
        setState(Running);
    }
}

void RsyncProcess::cancelBackup()
{
    if (m_state == Idle || m_state == Finished) return;

    if (m_state == Paused) {
        resumeBackup(); // 終了シグナルを受け取れるように再開
    }

    m_process.disconnect(this);
    qint64 pid = m_process.processId();

    m_process.terminate();
    if (!m_process.waitForFinished(1000)) {
        if (pid > 0) {
            ::kill(-static_cast<pid_t>(pid), SIGKILL);
        }
        m_process.kill();
        m_process.waitForFinished(500);
    }

    connect(&m_process, &QProcess::readyReadStandardOutput, this, &RsyncProcess::onReadyReadStandardOutput, Qt::UniqueConnection);
    connect(&m_process, &QProcess::readyReadStandardError, this, &RsyncProcess::onReadyReadStandardError, Qt::UniqueConnection);
    connect(&m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &RsyncProcess::onProcessFinished, Qt::UniqueConnection);

    setState(Idle);
}

qint64 RsyncProcess::currentPid() const
{
    return m_process.processId();
}

void RsyncProcess::setState(State state)
{
    if (m_state != state) {
        m_state = state;
        emit stateChanged(m_state);
    }
}

void RsyncProcess::onReadyReadStandardOutput()
{
    QByteArray data = m_process.readAllStandardOutput();
    if (data.isEmpty()) return;

    QString text = QString::fromUtf8(data);
    m_buffer += text;

    // メモリ保護のためバッファサイズの上限チェック (64KB)
    if (m_buffer.size() > 65536) {
        m_buffer = m_buffer.right(32768);
    }

    int pos = 0;
    while ((pos = m_buffer.indexOf(QRegularExpression("[\r\n]"))) != -1) {
        QString line = m_buffer.left(pos).trimmed();
        m_buffer = m_buffer.mid(pos + 1);

        if (!line.isEmpty()) {
            if (!parseProgressLine(line)) {
                emit logOutput(line);
            }
        }
    }
}

void RsyncProcess::onReadyReadStandardError()
{
    QByteArray data = m_process.readAllStandardError();
    if (data.isEmpty()) return;

    QString errText = QString::fromUtf8(data);
    m_lastError += errText;
    emit logOutput("[ERROR] " + errText.trimmed());
}

void RsyncProcess::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    bool success = (exitStatus == QProcess::NormalExit && (exitCode == 0 || exitCode == 24));

    if (success) {
        setState(Finished);
    } else {
        if (m_lastError.isEmpty()) {
            m_lastError = QString("rsync exited with code %1").arg(exitCode);
        }
        setState(Error);
    }

    emit finished(exitCode, success);
}

bool RsyncProcess::parseProgressLine(const QString &line)
{
    // 例: "  1,234,567  45%   12.34MB/s    0:00:15 (xfr#12, to-chk=45/100)"
    static const QRegularExpression re(R"(\s*([\d,]+)\s+(\d+)%\s+([\d\.\w/]+)\s+([\d:]+))");
    QRegularExpressionMatch match = re.match(line);

    if (match.hasMatch()) {
        QString transferred = match.captured(1);
        int percent = match.captured(2).toInt();
        QString speed = match.captured(3);
        QString remainingTime = match.captured(4);

        emit progressUpdated(percent, speed, transferred, remainingTime);
        return true;
    }
    return false;
}
