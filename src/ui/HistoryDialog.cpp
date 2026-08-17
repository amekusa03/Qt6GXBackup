#include "HistoryDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QDateTime>

HistoryDialog::HistoryDialog(HistoryManager *historyManager, QWidget *parent)
    : QDialog(parent), m_historyManager(historyManager)
{
    setupUi();
    refreshHistory();

    if (m_historyManager) {
        connect(m_historyManager, &HistoryManager::historyChanged, this, &HistoryDialog::refreshHistory);
    }
}

void HistoryDialog::setupUi()
{
    setWindowTitle("バックアップ実行履歴 - GXBackup");
    resize(720, 450);

    QVBoxLayout *layout = new QVBoxLayout(this);

    m_tableWidget = new QTableWidget(this);
    m_tableWidget->setColumnCount(5);
    m_tableWidget->setHorizontalHeaderLabels({"プロファイル名", "開始日時", "終了日時", "結果", "詳細"});
    m_tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_tableWidget->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);
    m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    layout->addWidget(m_tableWidget);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    m_refreshBtn = new QPushButton("更新", this);
    m_closeBtn = new QPushButton("閉じる", this);

    btnLayout->addWidget(m_refreshBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(m_closeBtn);

    layout->addLayout(btnLayout);

    connect(m_refreshBtn, &QPushButton::clicked, this, &HistoryDialog::refreshHistory);
    connect(m_closeBtn, &QPushButton::clicked, this, &QDialog::accept);
}

void HistoryDialog::refreshHistory()
{
    if (!m_historyManager) return;

    m_tableWidget->setUpdatesEnabled(false);
    m_tableWidget->setRowCount(0);
    auto historyList = m_historyManager->history();

    for (const auto &rec : historyList) {
        int row = m_tableWidget->rowCount();
        m_tableWidget->insertRow(row);

        m_tableWidget->setItem(row, 0, new QTableWidgetItem(rec.profileName));
        m_tableWidget->setItem(row, 1, new QTableWidgetItem(rec.startDateTime));
        m_tableWidget->setItem(row, 2, new QTableWidgetItem(rec.endDateTime));

        QTableWidgetItem *statusItem = new QTableWidgetItem(rec.success ? "🟢 成功" : "🔴 失敗");
        m_tableWidget->setItem(row, 3, statusItem);
        m_tableWidget->setItem(row, 4, new QTableWidgetItem(rec.detailMessage));
    }

    m_tableWidget->setUpdatesEnabled(true);
}
