#include "LogViewer.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFontDatabase>

LogViewer::LogViewer(QWidget *parent)
    : QDialog(parent)
{
    setupUi();
    retranslateUi();
}

void LogViewer::setupUi()
{
    resize(700, 450);

    QVBoxLayout *layout = new QVBoxLayout(this);

    m_textEdit = new QTextEdit(this);
    m_textEdit->setReadOnly(true);
    m_textEdit->document()->setMaximumBlockCount(2000);
    m_textEdit->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    layout->addWidget(m_textEdit);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    m_clearBtn = new QPushButton(this);
    m_closeBtn = new QPushButton(this);
    btnLayout->addWidget(m_clearBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(m_closeBtn);

    layout->addLayout(btnLayout);

    connect(m_clearBtn, &QPushButton::clicked, this, &LogViewer::clearLog);
    connect(m_closeBtn, &QPushButton::clicked, this, &QDialog::accept);
}

void LogViewer::retranslateUi()
{
    setWindowTitle(tr("Execution Log - GXBackup"));
    m_clearBtn->setText(tr("Clear Log"));
    m_closeBtn->setText(tr("Close"));
}

void LogViewer::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QDialog::changeEvent(event);
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
