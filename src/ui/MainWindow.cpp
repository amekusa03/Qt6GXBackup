#include "MainWindow.h"
#include "ProfileDialog.h"
#include "AutostartManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QHeaderView>
#include <QDir>
#include <QApplication>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    m_backupController = new BackupController(&m_historyManager, this);

    setupUi();
    applyStyleSheet();
    setupTrayIcon();

    m_logViewer = new LogViewer(this);

    m_logFlushTimer = new QTimer(this);
    connect(m_logFlushTimer, &QTimer::timeout, this, &MainWindow::flushLogBuffer);
    m_logFlushTimer->setInterval(100);

    // バックアップコントローラーのシグナル接続
    connect(m_backupController, &BackupController::stateChanged, this, &MainWindow::onStateChanged);
    connect(m_backupController, &BackupController::progressUpdated, this, &MainWindow::onProgressUpdated);
    connect(m_backupController, &BackupController::logMessage, this, &MainWindow::onLogMessage);
    connect(m_backupController, &BackupController::backupFinished, this, &MainWindow::onBackupFinished);
    connect(m_backupController, &BackupController::loadWarning, this, &MainWindow::onLoadWarning);

    connect(m_backupController->monitor(), &SystemMonitor::metricsUpdated, this, &MainWindow::onMetricsUpdated);

    // スケジュールマネージャー初期化
    m_scheduleManager = new ScheduleManager(&m_profileManager, m_backupController, &m_historyManager, this);
    connect(m_scheduleManager, &ScheduleManager::scheduledBackupTriggered, this, &MainWindow::onScheduledBackupTriggered);
    connect(m_scheduleManager, &ScheduleManager::scheduleDelayed, this, &MainWindow::onScheduleDelayed);
    m_scheduleManager->start();

    updateProfileCombo();
    m_backupController->monitor()->startMonitoring(2000);
}

void MainWindow::setupUi()
{
    setWindowTitle("GXBackup - Linux Smart Backup Tool");
    resize(880, 640);

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(12);

    // --- 上部: プロファイル選択エリア ---
    QGroupBox *profileGroup = new QGroupBox("バックアッププロファイル", this);
    QHBoxLayout *profileLayout = new QHBoxLayout(profileGroup);

    m_profileCombo = new QComboBox(this);
    m_newBtn = new QPushButton("新規...", this);
    m_editBtn = new QPushButton("編集...", this);
    m_deleteBtn = new QPushButton("削除", this);
    m_historyBtn = new QPushButton("📜 履歴", this);

    profileLayout->addWidget(m_profileCombo, 1);
    profileLayout->addWidget(m_newBtn);
    profileLayout->addWidget(m_editBtn);
    profileLayout->addWidget(m_deleteBtn);
    profileLayout->addWidget(m_historyBtn);

    mainLayout->addWidget(profileGroup);

    connect(m_newBtn, &QPushButton::clicked, this, &MainWindow::onNewProfile);
    connect(m_editBtn, &QPushButton::clicked, this, &MainWindow::onEditProfile);
    connect(m_deleteBtn, &QPushButton::clicked, this, &MainWindow::onDeleteProfile);
    connect(m_historyBtn, &QPushButton::clicked, this, &MainWindow::onOpenHistory);
    connect(m_profileCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onProfileSelected);

    // --- 中央: ステータス & 負荷モニター (2カラムレイアウト) ---
    QHBoxLayout *middleLayout = new QHBoxLayout();

    // 1. バックアップステータスカード
    QGroupBox *statusGroup = new QGroupBox("進捗状況", this);
    QVBoxLayout *statusLayout = new QVBoxLayout(statusGroup);

    m_statusLabel = new QLabel("待機中", this);
    QFont statusFont = m_statusLabel->font();
    statusFont.setBold(true);
    statusFont.setPointSizeF(statusFont.pointSizeF() * 1.2);
    m_statusLabel->setFont(statusFont);

    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);

    QGridLayout *infoGrid = new QGridLayout();
    infoGrid->addWidget(new QLabel("転送速度:", this), 0, 0);
    m_speedLabel = new QLabel("- MB/s", this);
    infoGrid->addWidget(m_speedLabel, 0, 1);

    infoGrid->addWidget(new QLabel("転送量:", this), 1, 0);
    m_transferredLabel = new QLabel("0 B", this);
    infoGrid->addWidget(m_transferredLabel, 1, 1);

    infoGrid->addWidget(new QLabel("残り時間:", this), 2, 0);
    m_remainingLabel = new QLabel("--:--:--", this);
    infoGrid->addWidget(m_remainingLabel, 2, 1);

    statusLayout->addWidget(m_statusLabel);
    statusLayout->addWidget(m_progressBar);
    statusLayout->addLayout(infoGrid);

    // 2. システムリソースモニターカード
    QGroupBox *monitorGroup = new QGroupBox("システム負荷状況 (Smart Monitor)", this);
    QVBoxLayout *monitorLayout = new QVBoxLayout(monitorGroup);

    monitorLayout->addWidget(new QLabel("CPU使用率:", this));
    m_cpuBar = new QProgressBar(this);
    m_cpuBar->setRange(0, 100);
    m_cpuBar->setValue(0);
    monitorLayout->addWidget(m_cpuBar);

    QHBoxLayout *loadLayout = new QHBoxLayout();
    loadLayout->addWidget(new QLabel("Load Average (1分):", this));
    m_loadAvgLabel = new QLabel("0.00", this);
    loadLayout->addWidget(m_loadAvgLabel);
    monitorLayout->addLayout(loadLayout);

    m_autoPauseIndicator = new QLabel("🟢 正常動作中 (自動一時停止OFF)", this);
    m_autoPauseIndicator->setWordWrap(true);
    monitorLayout->addWidget(m_autoPauseIndicator);
    monitorLayout->addStretch();

    middleLayout->addWidget(statusGroup, 3);
    middleLayout->addWidget(monitorGroup, 2);
    mainLayout->addLayout(middleLayout);

    // --- 下部: 操作ボタンとログ表示 ---
    QHBoxLayout *ctrlLayout = new QHBoxLayout();
    m_startBtn = new QPushButton("▶ バックアップ開始", this);
    m_pauseBtn = new QPushButton("⏸ 一時停止", this);
    m_cancelBtn = new QPushButton("⏹ キャンセル", this);

    m_startBtn->setMinimumHeight(40);
    m_pauseBtn->setMinimumHeight(40);
    m_cancelBtn->setMinimumHeight(40);
    m_pauseBtn->setEnabled(false);
    m_cancelBtn->setEnabled(false);

    ctrlLayout->addWidget(m_startBtn, 2);
    ctrlLayout->addWidget(m_pauseBtn, 1);
    ctrlLayout->addWidget(m_cancelBtn, 1);

    mainLayout->addLayout(ctrlLayout);

    connect(m_startBtn, &QPushButton::clicked, this, &MainWindow::onStartBackup);
    connect(m_pauseBtn, &QPushButton::clicked, this, &MainWindow::onPauseResumeBackup);
    connect(m_cancelBtn, &QPushButton::clicked, this, &MainWindow::onCancelBackup);

    // ログエリア
    QGroupBox *logGroup = new QGroupBox("ログ出力", this);
    QVBoxLayout *logLayout = new QVBoxLayout(logGroup);
    m_logEdit = new QTextEdit(this);
    m_logEdit->document()->setMaximumBlockCount(1000);
    m_logEdit->setReadOnly(true);
    m_logEdit->setMaximumHeight(130);
    logLayout->addWidget(m_logEdit);

    mainLayout->addWidget(logGroup);
}

void MainWindow::setupTrayIcon()
{
    m_trayIcon = new QSystemTrayIcon(this);
    QIcon appIcon = QIcon::fromTheme("system-file-manager", QApplication::style()->standardIcon(QStyle::SP_DriveHDIcon));
    m_trayIcon->setIcon(appIcon);
    m_trayIcon->setToolTip("GXBackup - Linux Smart Backup Tool");

    m_trayMenu = new QMenu(this);

    QAction *restoreAction = m_trayMenu->addAction("📂 メイン画面を開く");
    connect(restoreAction, &QAction::triggered, this, [this]() {
        this->showNormal();
        this->activateWindow();
    });

    QAction *startAction = m_trayMenu->addAction("▶ 今すぐバックアップ実行");
    connect(startAction, &QAction::triggered, this, &MainWindow::onStartBackup);

    QAction *historyAction = m_trayMenu->addAction("📜 履歴を見る");
    connect(historyAction, &QAction::triggered, this, &MainWindow::onOpenHistory);

    m_trayMenu->addSeparator();

    m_autostartAction = m_trayMenu->addAction("⚙ ログイン時に自動起動");
    m_autostartAction->setCheckable(true);
    m_autostartAction->setChecked(AutostartManager::isAutostartEnabled());
    connect(m_autostartAction, &QAction::toggled, this, &MainWindow::onToggleAutostart);

    m_trayMenu->addSeparator();

    QAction *quitAction = m_trayMenu->addAction("🚪 完全に終了");
    connect(quitAction, &QAction::triggered, this, [this]() {
        m_forceQuit = true;
        qApp->quit();
    });

    m_trayIcon->setContextMenu(m_trayMenu);
    connect(m_trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::onTrayIconActivated);

    m_trayIcon->show();
}

void MainWindow::setStartMinimized(bool minimized)
{
    if (minimized) {
        hide();
        if (m_trayIcon) {
            m_trayIcon->showMessage("GXBackup", "バックグラウンドでシステムトレイに常駐しました。", QSystemTrayIcon::Information, 3000);
        }
    } else {
        show();
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (!m_forceQuit && m_trayIcon->isVisible()) {
        event->ignore();
        this->hide();
        m_trayIcon->showMessage("GXBackup 常駐中",
                                "バックグラウンドでバックアップの監視を行っています。\n終了するにはトレイアイコンから「完全に終了」を選択してください。",
                                QSystemTrayIcon::Information, 3000);
    } else {
        event->accept();
    }
}

void MainWindow::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick) {
        if (isVisible()) {
            hide();
        } else {
            showNormal();
            activateWindow();
        }
    }
}

void MainWindow::onToggleAutostart(bool checked)
{
    bool ok = AutostartManager::setAutostartEnabled(checked);
    if (ok) {
        if (checked) {
            m_trayIcon->showMessage("自動起動設定", "Linuxログイン時の自動起動を有効にしました。", QSystemTrayIcon::Information, 3000);
        } else {
            m_trayIcon->showMessage("自動起動設定", "自動起動を解除しました。", QSystemTrayIcon::Information, 3000);
        }
    } else {
        QMessageBox::warning(this, "エラー", "自動起動設定の更新に失敗しました。");
        m_autostartAction->setChecked(!checked);
    }
}

void MainWindow::onOpenHistory()
{
    HistoryDialog dialog(&m_historyManager, this);
    dialog.exec();
}

void MainWindow::onScheduledBackupTriggered(const BackupProfile &profile, bool isCatchUp)
{
    int idx = m_profileCombo->findData(profile.id);
    if (idx != -1) {
        m_profileCombo->setCurrentIndex(idx);
    }
    QString msg = isCatchUp 
        ? QString("未実行のスケジュール条件を満たしたため、プロファイル「%1」の追いつきバックアップを開始しました。").arg(profile.name)
        : QString("定時スケジュールに従い、プロファイル「%1」の自動バックアップを開始しました。").arg(profile.name);

    m_trayIcon->showMessage(isCatchUp ? "未実行分の追いつきバックアップ開始" : "スケジュールバックアップ開始",
                            msg, QSystemTrayIcon::Information, 5000);
}

void MainWindow::onScheduleDelayed(const QString &profileName, const QString &reason)
{
    m_trayIcon->showMessage("スケジュール実行延期",
                            QString("「%1」: %2").arg(profileName, reason),
                            QSystemTrayIcon::Warning, 4000);
}

void MainWindow::applyStyleSheet()
{
    QString qss = R"(
        QMainWindow {
            background-color: #1e1e2e;
            color: #cdd6f4;
        }
        QGroupBox {
            font-weight: bold;
            border: 1px solid #45475a;
            border-radius: 8px;
            margin-top: 10px;
            padding-top: 10px;
            color: #89b4fa;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px;
        }
        QPushButton {
            background-color: #313244;
            color: #cdd6f4;
            border: 1px solid #45475a;
            border-radius: 6px;
            padding: 6px 12px;
        }
        QPushButton:hover {
            background-color: #45475a;
        }
        QPushButton:pressed {
            background-color: #585b70;
        }
        QPushButton#startBtn {
            background-color: #a6e3a1;
            color: #11111b;
            font-weight: bold;
        }
        QPushButton#startBtn:hover {
            background-color: #94e2d5;
        }
        QProgressBar {
            border: 1px solid #45475a;
            border-radius: 4px;
            text-align: center;
            background-color: #181825;
            color: #cdd6f4;
        }
        QProgressBar::chunk {
            background-color: #89b4fa;
            border-radius: 3px;
        }
        QTextEdit, QComboBox {
            background-color: #181825;
            color: #cdd6f4;
            border: 1px solid #45475a;
            border-radius: 4px;
        }
    )";
    this->setStyleSheet(qss);
    m_startBtn->setObjectName("startBtn");
}

void MainWindow::updateProfileCombo()
{
    m_profileCombo->clear();
    auto profiles = m_profileManager.profiles();

    if (profiles.isEmpty()) {
        // 初期プロファイルが無い場合はプリセットを作成
        BackupProfile userP = ProfileManager::createUserDataPreset(QDir::homePath() + "/Backups/user_data");
        m_profileManager.addProfile(userP);
        profiles = m_profileManager.profiles();
    }

    for (const auto &p : profiles) {
        QString itemText = p.name;
        if (p.scheduleEnabled) {
            itemText += QString(" [📅 %1 %2]").arg(p.scheduleType == "daily" ? "毎日" : "毎週", p.scheduleTime);
        }
        m_profileCombo->addItem(itemText, p.id);
    }
}

BackupProfile MainWindow::currentSelectedProfile() const
{
    QString id = m_profileCombo->currentData().toString();
    return m_profileManager.profile(id);
}

void MainWindow::onNewProfile()
{
    ProfileDialog dialog(BackupProfile(), this);
    if (dialog.exec() == QDialog::Accepted) {
        m_profileManager.addProfile(dialog.profile());
        updateProfileCombo();
    }
}

void MainWindow::onEditProfile()
{
    BackupProfile p = currentSelectedProfile();
    if (p.id.isEmpty()) return;

    ProfileDialog dialog(p, this);
    if (dialog.exec() == QDialog::Accepted) {
        m_profileManager.updateProfile(dialog.profile());
        updateProfileCombo();
    }
}

void MainWindow::onDeleteProfile()
{
    BackupProfile p = currentSelectedProfile();
    if (p.id.isEmpty()) return;

    if (QMessageBox::question(this, "削除確認", QString("プロファイル「%1」を削除しますか？").arg(p.name)) == QMessageBox::Yes) {
        m_profileManager.removeProfile(p.id);
        updateProfileCombo();
    }
}

void MainWindow::onProfileSelected(int index)
{
    Q_UNUSED(index);
    BackupProfile p = currentSelectedProfile();
    if (p.id.isEmpty()) return;

    if (p.autoPauseOnHighLoad) {
        m_autoPauseIndicator->setText(QString("🟢 自動一時停止有効 (CPU > %1%, Load > %2)")
                                      .arg(p.cpuThreshold, 0, 'f', 0)
                                      .arg(p.loadAvgThreshold, 0, 'f', 1));
    } else {
        m_autoPauseIndicator->setText("⚪ 自動一時停止OFF");
    }
}

void MainWindow::onStartBackup()
{
    BackupProfile p = currentSelectedProfile();
    if (p.id.isEmpty()) {
        QMessageBox::warning(this, "エラー", "プロファイルを選択してください。");
        return;
    }

    m_startBtn->setEnabled(false);
    m_pauseBtn->setEnabled(true);
    m_cancelBtn->setEnabled(true);
    m_profileCombo->setEnabled(false);

    m_backupController->startBackup(p);
}

void MainWindow::onPauseResumeBackup()
{
    if (m_backupController->state() == RsyncProcess::Running) {
        m_backupController->pauseBackup();
        m_pauseBtn->setText("▶ 再開");
    } else if (m_backupController->state() == RsyncProcess::Paused) {
        m_backupController->resumeBackup();
        m_pauseBtn->setText("⏸ 一時停止");
    }
}

void MainWindow::onCancelBackup()
{
    m_backupController->cancelBackup();
}

void MainWindow::onStateChanged(RsyncProcess::State state)
{
    switch (state) {
    case RsyncProcess::Idle:
        m_statusLabel->setText("待機中");
        m_startBtn->setEnabled(true);
        m_pauseBtn->setEnabled(false);
        m_cancelBtn->setEnabled(false);
        m_profileCombo->setEnabled(true);
        m_pauseBtn->setText("⏸ 一時停止");
        break;
    case RsyncProcess::Running:
        m_statusLabel->setText("バックアップ実行中...");
        m_pauseBtn->setText("⏸ 一時停止");
        m_startBtn->setEnabled(false);
        m_pauseBtn->setEnabled(true);
        m_cancelBtn->setEnabled(true);
        m_profileCombo->setEnabled(false);
        break;
    case RsyncProcess::Paused:
        m_statusLabel->setText("一時停止中 (高負荷または手動)");
        m_pauseBtn->setText("▶ 再開");
        m_startBtn->setEnabled(false);
        m_pauseBtn->setEnabled(true);
        m_cancelBtn->setEnabled(true);
        m_profileCombo->setEnabled(false);
        break;
    case RsyncProcess::Finished:
        m_statusLabel->setText("完了");
        m_startBtn->setEnabled(true);
        m_pauseBtn->setEnabled(false);
        m_cancelBtn->setEnabled(false);
        m_profileCombo->setEnabled(true);
        m_pauseBtn->setText("⏸ 一時停止");
        break;
    case RsyncProcess::Error:
        m_statusLabel->setText("エラー発生");
        m_startBtn->setEnabled(true);
        m_pauseBtn->setEnabled(false);
        m_cancelBtn->setEnabled(false);
        m_profileCombo->setEnabled(true);
        m_pauseBtn->setText("⏸ 一時停止");
        break;
    }
}

void MainWindow::onProgressUpdated(int percent, QString speed, QString transferred, QString remainingTime)
{
    m_progressBar->setValue(percent);
    m_speedLabel->setText(speed);
    m_transferredLabel->setText(transferred);
    m_remainingLabel->setText(remainingTime);
}

void MainWindow::onLogMessage(const QString &msg)
{
    m_logBuffer.append(msg);
    if (m_logBuffer.size() >= 100) {
        flushLogBuffer();
    } else if (m_logFlushTimer && !m_logFlushTimer->isActive()) {
        m_logFlushTimer->start();
    }
}

void MainWindow::flushLogBuffer()
{
    if (m_logBuffer.isEmpty()) {
        if (m_logFlushTimer && m_logFlushTimer->isActive()) {
            m_logFlushTimer->stop();
        }
        return;
    }
    QString joinedLogs = m_logBuffer.join('\n');
    m_logEdit->append(joinedLogs);
    if (m_logViewer) {
        m_logViewer->appendLogs(m_logBuffer);
    }
    m_logBuffer.clear();
    if (m_logFlushTimer && m_logFlushTimer->isActive()) {
        m_logFlushTimer->stop();
    }
}

void MainWindow::onBackupFinished(bool success, const QString &message)
{
    flushLogBuffer();
    m_startBtn->setEnabled(true);
    m_pauseBtn->setEnabled(false);
    m_cancelBtn->setEnabled(false);
    m_profileCombo->setEnabled(true);
    m_pauseBtn->setText("⏸ 一時停止");

    if (success) {
        if (m_trayIcon) {
            m_trayIcon->showMessage("バックアップ完了", message, QSystemTrayIcon::Information, 5000);
        }
        if (isVisible()) {
            QMessageBox::information(this, "完了", message);
        }
    } else {
        if (m_trayIcon) {
            m_trayIcon->showMessage("バックアップエラー", message, QSystemTrayIcon::Critical, 5000);
        }
        if (isVisible()) {
            QMessageBox::critical(this, "エラー", message);
        }
    }
}

void MainWindow::onLoadWarning(const QString &warningMsg)
{
    m_autoPauseIndicator->setText("🟡 " + warningMsg);
    if (m_trayIcon) {
        m_trayIcon->showMessage("スマート負荷制御", warningMsg, QSystemTrayIcon::Warning, 4000);
    }
}

void MainWindow::onMetricsUpdated(double cpuPercent, double loadAvg)
{
    m_cpuBar->setValue(static_cast<int>(cpuPercent));
    m_loadAvgLabel->setText(QString::number(loadAvg, 'f', 2));
}
