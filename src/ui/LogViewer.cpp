#include "LogViewer.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFontDatabase>

LogViewer::LogViewer(QWidget *parent)
    : QDialog(parent)
{
    setupUi();
}

void LogViewer::setupUi()
{
    setWindowTitle("実行ログ - GXBackup");
    resize(700, 450);

    QVBoxLayout *layout = new QVBoxLayout(this);

    m_textEdit = new QTextEdit(this);
    m_textEdit->setReadOnly(true);
    m_textEdit->document()->setMaximumBlockCount(2000);
    m_textEdit->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    layout->addWidget(m_textEdit);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    m_clearBtn = new QPushButton("ログ消去", this);
    m_closeBtn = new QPushButton("閉じる", this);
    btnLayout->addWidget(m_clearBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(m_closeBtn);

    layout->addLayout(btnLayout);

    connect(m_clearBtn, &QPushButton::clicked, this, &LogViewer::clearLog);
    connect(m_closeBtn, &QPushButton::clicked, this, &QDialog::accept);
}

void LogViewer::appendLog(const QString &line)
{
    m_textEdit->append(line);
}

void LogViewer::appendLogs(const QStringList &lines)
{
    if (!lines.isEmpty()) {
        m_textEdit->append(lines.join('\n'));
    }
}

void LogViewer::clearLog()
{
    m_textEdit->clear();
}
