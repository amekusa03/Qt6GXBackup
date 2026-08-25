#include "MainWindow.h"
#include "ProfileDialog.h"
#include "AutostartManager.h"
#include "LanguageManager.h"
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
    setupMenuBar();
    applyStyleSheet();
    setupTrayIcon();

    m_logViewer = new LogViewer(this);

    m_logFlushTimer = new QTimer(this);
    connect(m_logFlushTimer, &QTimer::timeout, this, &MainWindow::flushLogBuffer);
    m_logFlushTimer->setInterval(100);

    // Connect BackupController signals
    connect(m_backupController, &BackupController::stateChanged, this, &MainWindow::onStateChanged);
    connect(m_backupController, &BackupController::progressUpdated, this, &MainWindow::onProgressUpdated);
    connect(m_backupController, &BackupController::logMessage, this, &MainWindow::onLogMessage);
    connect(m_backupController, &BackupController::backupFinished, this, &MainWindow::onBackupFinished);
    connect(m_backupController, &BackupController::loadWarning, this, &MainWindow::onLoadWarning);

    connect(m_backupController->monitor(), &SystemMonitor::metricsUpdated, this, &MainWindow::onMetricsUpdated);

    // Initialize ScheduleManager
    m_scheduleManager = new ScheduleManager(&m_profileManager, m_backupController, &m_historyManager, this);
    connect(m_scheduleManager, &ScheduleManager::scheduledBackupTriggered, this, &MainWindow::onScheduledBackupTriggered);
    connect(m_scheduleManager, &ScheduleManager::scheduleDelayed, this, &MainWindow::onScheduleDelayed);
    m_scheduleManager->start();

    retranslateUi();
    updateProfileCombo();
    m_backupController->monitor()->startMonitoring(2000);
}

void MainWindow::setupUi()
{
    resize(880, 640);

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(12);

    // --- Top: Profile selection area ---
    m_profileGroup = new QGroupBox(this);
    QHBoxLayout *profileLayout = new QHBoxLayout(m_profileGroup);

    m_profileCombo = new QComboBox(this);
    m_newBtn = new QPushButton(this);
    m_editBtn = new QPushButton(this);
    m_deleteBtn = new QPushButton(this);
    m_historyBtn = new QPushButton(this);

    profileLayout->addWidget(m_profileCombo, 1);
    profileLayout->addWidget(m_newBtn);
    profileLayout->addWidget(m_editBtn);
    profileLayout->addWidget(m_deleteBtn);
    profileLayout->addWidget(m_historyBtn);

    mainLayout->addWidget(m_profileGroup);

    connect(m_newBtn, &QPushButton::clicked, this, &MainWindow::onNewProfile);
    connect(m_editBtn, &QPushButton::clicked, this, &MainWindow::onEditProfile);
    connect(m_deleteBtn, &QPushButton::clicked, this, &MainWindow::onDeleteProfile);
    connect(m_historyBtn, &QPushButton::clicked, this, &MainWindow::onOpenHistory);
    connect(m_profileCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onProfileSelected);

    // --- Center: Status & Resource Monitor (2-column layout) ---
    QHBoxLayout *middleLayout = new QHBoxLayout();

    // 1. Backup Status Card
    m_statusGroup = new QGroupBox(this);
    QVBoxLayout *statusLayout = new QVBoxLayout(m_statusGroup);

    m_statusLabel = new QLabel(this);
    QFont statusFont = m_statusLabel->font();
    statusFont.setBold(true);
    statusFont.setPointSizeF(statusFont.pointSizeF() * 1.2);
    m_statusLabel->setFont(statusFont);

    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);

    QGridLayout *infoGrid = new QGridLayout();
    m_speedTitleLabel = new QLabel(this);
    infoGrid->addWidget(m_speedTitleLabel, 0, 0);
    m_speedLabel = new QLabel("- MB/s", this);
    infoGrid->addWidget(m_speedLabel, 0, 1);

    m_transferredTitleLabel = new QLabel(this);
    infoGrid->addWidget(m_transferredTitleLabel, 1, 0);
    m_transferredLabel = new QLabel("0 B", this);
    infoGrid->addWidget(m_transferredLabel, 1, 1);

    m_remainingTitleLabel = new QLabel(this);
    infoGrid->addWidget(m_remainingTitleLabel, 2, 0);
    m_remainingLabel = new QLabel("--:--:--", this);
    infoGrid->addWidget(m_remainingLabel, 2, 1);

    statusLayout->addWidget(m_statusLabel);
    statusLayout->addWidget(m_progressBar);
    statusLayout->addLayout(infoGrid);

    // 2. System Resource Monitor Card
    m_monitorGroup = new QGroupBox(this);
    QVBoxLayout *monitorLayout = new QVBoxLayout(m_monitorGroup);

    m_cpuTitleLabel = new QLabel(this);
    monitorLayout->addWidget(m_cpuTitleLabel);
    m_cpuBar = new QProgressBar(this);
    m_cpuBar->setRange(0, 100);
    m_cpuBar->setValue(0);
    monitorLayout->addWidget(m_cpuBar);

    QHBoxLayout *loadLayout = new QHBoxLayout();
    m_loadAvgTitleLabel = new QLabel(this);
    loadLayout->addWidget(m_loadAvgTitleLabel);
    m_loadAvgLabel = new QLabel("0.00", this);
    loadLayout->addWidget(m_loadAvgLabel);
    monitorLayout->addLayout(loadLayout);

    m_autoPauseIndicator = new QLabel(this);
    m_autoPauseIndicator->setWordWrap(true);
    monitorLayout->addWidget(m_autoPauseIndicator);
    monitorLayout->addStretch();

    middleLayout->addWidget(m_statusGroup, 3);
    middleLayout->addWidget(m_monitorGroup, 2);
    mainLayout->addLayout(middleLayout);

    // --- Bottom: Control buttons & Log display ---
    QHBoxLayout *ctrlLayout = new QHBoxLayout();
    m_startBtn = new QPushButton(this);
    m_pauseBtn = new QPushButton(this);
    m_cancelBtn = new QPushButton(this);

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

    // Log area
    m_logGroup = new QGroupBox(this);
    QVBoxLayout *logLayout = new QVBoxLayout(m_logGroup);
    m_logEdit = new QTextEdit(this);
    m_logEdit->document()->setMaximumBlockCount(1000);
    m_logEdit->setReadOnly(true);
    m_logEdit->setMaximumHeight(130);
    logLayout->addWidget(m_logEdit);

    mainLayout->addWidget(m_logGroup);
}

void MainWindow::setupMenuBar()
{
    m_settingsMenu = menuBar()->addMenu("");
    m_languageMenu = m_settingsMenu->addMenu("");

    m_langActionGroup = new QActionGroup(this);
    m_langActionGroup->setExclusive(true);

    m_englishAction = m_languageMenu->addAction("English");
    m_englishAction->setCheckable(true);
    m_englishAction->setData("en");
    m_langActionGroup->addAction(m_englishAction);

    m_japaneseAction = m_languageMenu->addAction("日本語 (Japanese)");
    m_japaneseAction->setCheckable(true);
    m_japaneseAction->setData("ja");
    m_langActionGroup->addAction(m_japaneseAction);

    connect(m_langActionGroup, &QActionGroup::triggered, this, &MainWindow::onLanguageSelected);
}

void MainWindow::onLanguageSelected(QAction *action)
{
    if (!action) return;
    QString langCode = action->data().toString();
    LanguageManager::instance().setLanguage(langCode);
}

void MainWindow::retranslateUi()
{
    setWindowTitle(tr("GXBackup - Linux Smart Backup Tool"));

    if (m_settingsMenu) m_settingsMenu->setTitle(tr("&Settings"));
    if (m_languageMenu) m_languageMenu->setTitle(tr("&Language"));

    QString currentLang = LanguageManager::instance().currentLanguage();
    if (m_englishAction) m_englishAction->setChecked(currentLang == "en");
    if (m_japaneseAction) m_japaneseAction->setChecked(currentLang == "ja");

    m_profileGroup->setTitle(tr("Backup Profiles"));
    m_newBtn->setText(tr("New..."));
    m_editBtn->setText(tr("Edit..."));
    m_deleteBtn->setText(tr("Delete"));
    m_historyBtn->setText(tr("📜 History"));

    m_statusGroup->setTitle(tr("Progress Status"));
    m_speedTitleLabel->setText(tr("Transfer Speed:"));
    m_transferredTitleLabel->setText(tr("Transferred:"));
    m_remainingTitleLabel->setText(tr("Remaining Time:"));

    m_monitorGroup->setTitle(tr("System Resource Monitor (Smart Monitor)"));
    m_cpuTitleLabel->setText(tr("CPU Usage:"));
    m_loadAvgTitleLabel->setText(tr("Load Average (1m):"));

    m_startBtn->setText(tr("▶ Start Backup"));
    m_cancelBtn->setText(tr("⏹ Cancel"));

    RsyncProcess::State currentState = m_backupController ? m_backupController->state() : RsyncProcess::Idle;
    if (currentState == RsyncProcess::Paused) {
        m_pauseBtn->setText(tr("▶ Resume"));
    } else {
        m_pauseBtn->setText(tr("⏸ Pause"));
    }

    switch (currentState) {
    case RsyncProcess::Idle:
        m_statusLabel->setText(tr("Idle"));
        break;
    case RsyncProcess::Running:
        m_statusLabel->setText(tr("Backup in progress..."));
        break;
    case RsyncProcess::Paused:
        m_statusLabel->setText(tr("Paused (High load or manual)"));
        break;
    case RsyncProcess::Finished:
        m_statusLabel->setText(tr("Completed"));
        break;
    case RsyncProcess::Error:
        m_statusLabel->setText(tr("Error occurred"));
        break;
    }

    m_logGroup->setTitle(tr("Log Output"));

    if (m_trayIcon) {
        m_trayIcon->setToolTip(tr("GXBackup - Linux Smart Backup Tool"));
    }
    if (m_trayOpenAction) m_trayOpenAction->setText(tr("📂 Open Main Window"));
    if (m_trayStartAction) m_trayStartAction->setText(tr("▶ Run Backup Now"));
    if (m_trayHistoryAction) m_trayHistoryAction->setText(tr("📜 View History"));
    if (m_autostartAction) m_autostartAction->setText(tr("⚙ Autostart on Login"));
    if (m_trayQuitAction) m_trayQuitAction->setText(tr("🚪 Quit"));

    onProfileSelected(m_profileCombo ? m_profileCombo->currentIndex() : 0);
    updateProfileCombo();
}

void MainWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QMainWindow::changeEvent(event);
}

void MainWindow::setupTrayIcon()
{
    m_trayIcon = new QSystemTrayIcon(this);
    QIcon appIcon = QIcon(":/icons/gxbackup.png");
    if (appIcon.isNull()) {
        appIcon = QIcon::fromTheme("system-file-manager", QApplication::style()->standardIcon(QStyle::SP_DriveHDIcon));
    }
    m_trayIcon->setIcon(appIcon);

    m_trayMenu = new QMenu(this);

    m_trayOpenAction = m_trayMenu->addAction("");
    connect(m_trayOpenAction, &QAction::triggered, this, [this]() {
        this->showNormal();
        this->activateWindow();
    });

    m_trayStartAction = m_trayMenu->addAction("");
    connect(m_trayStartAction, &QAction::triggered, this, &MainWindow::onStartBackup);

    m_trayHistoryAction = m_trayMenu->addAction("");
    connect(m_trayHistoryAction, &QAction::triggered, this, &MainWindow::onOpenHistory);

    m_trayMenu->addSeparator();

    m_autostartAction = m_trayMenu->addAction("");
    m_autostartAction->setCheckable(true);
    m_autostartAction->setChecked(AutostartManager::isAutostartEnabled());
    connect(m_autostartAction, &QAction::toggled, this, &MainWindow::onToggleAutostart);

    m_trayMenu->addSeparator();

    m_trayQuitAction = m_trayMenu->addAction("");
    connect(m_trayQuitAction, &QAction::triggered, this, [this]() {
        if (m_backupController && (m_backupController->state() == RsyncProcess::Running || m_backupController->state() == RsyncProcess::Paused)) {
            QMessageBox::StandardButton reply = QMessageBox::question(this, tr("Confirmation"),
                tr("Backup is running. Exiting will stop the backup. Are you sure you want to quit?"),
                QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
            if (reply != QMessageBox::Yes) {
                return;
            }
            m_backupController->cancelBackup();
        }
        m_forceQuit = true;
        qApp->quit();
    });

    m_trayIcon->setContextMenu(m_trayMenu);
    connect(m_trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::onTrayIconActivated);

    m_trayIcon->show();
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
            margin-top: 12px;
            padding-top: 12px;
            background-color: #181825;
            color: #cdd6f4;
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
    QString currentId = m_profileCombo->currentData().toString();
    m_profileCombo->blockSignals(true);
    m_profileCombo->clear();
    auto profiles = m_profileManager.profiles();

    if (profiles.isEmpty()) {
        // Create default preset profiles if none exist
        BackupProfile userP = ProfileManager::createUserDataPreset(QDir::homePath() + "/Backups/user_data");
        m_profileManager.addProfile(userP);
        profiles = m_profileManager.profiles();
    }

    int selectIndex = 0;
    for (int i = 0; i < profiles.size(); ++i) {
        const auto &p = profiles[i];
        QString itemText = p.name;
        if (p.scheduleEnabled) {
            itemText += QString(" [📅 %1 %2]").arg(p.scheduleType == "daily" ? tr("Daily") : tr("Weekly"), p.scheduleTime);
        }
        m_profileCombo->addItem(itemText, p.id);
        if (p.id == currentId) {
            selectIndex = i;
        }
    }
    m_profileCombo->setCurrentIndex(selectIndex);
    m_profileCombo->blockSignals(false);
}

void MainWindow::setProfileControlsEnabled(bool enabled)
{
    m_profileCombo->setEnabled(enabled);
    m_newBtn->setEnabled(enabled);
    m_editBtn->setEnabled(enabled);
    m_deleteBtn->setEnabled(enabled);
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

    if (QMessageBox::question(this, tr("Confirm Delete"), tr("Delete profile \"%1\"?").arg(p.name)) == QMessageBox::Yes) {
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
        m_autoPauseIndicator->setText(tr("🟢 Auto-pause active (CPU > %1%, Load > %2)")
                                      .arg(p.cpuThreshold, 0, 'f', 0)
                                      .arg(p.loadAvgThreshold, 0, 'f', 1));
    } else {
        m_autoPauseIndicator->setText(tr("⚪ Auto-pause OFF"));
    }
}

void MainWindow::onStartBackup()
{
    BackupProfile p = currentSelectedProfile();
    if (p.id.isEmpty()) {
        QMessageBox::warning(this, tr("Error"), tr("Please select a profile."));
        return;
    }

    m_startBtn->setEnabled(false);
    m_pauseBtn->setEnabled(true);
    m_cancelBtn->setEnabled(true);
    setProfileControlsEnabled(false);

    m_backupController->startBackup(p);
}

void MainWindow::onPauseResumeBackup()
{
    if (m_backupController->state() == RsyncProcess::Running) {
        m_backupController->pauseBackup();
        m_pauseBtn->setText(tr("▶ Resume"));
    } else if (m_backupController->state() == RsyncProcess::Paused) {
        m_backupController->resumeBackup();
        m_pauseBtn->setText(tr("⏸ Pause"));
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
        m_statusLabel->setText(tr("Idle"));
        m_startBtn->setEnabled(true);
        m_pauseBtn->setEnabled(false);
        m_cancelBtn->setEnabled(false);
        setProfileControlsEnabled(true);
        m_pauseBtn->setText(tr("⏸ Pause"));
        break;
    case RsyncProcess::Running:
        m_statusLabel->setText(tr("Backup in progress..."));
        m_pauseBtn->setText(tr("⏸ Pause"));
        m_startBtn->setEnabled(false);
        m_pauseBtn->setEnabled(true);
        m_cancelBtn->setEnabled(true);
        setProfileControlsEnabled(false);
        break;
    case RsyncProcess::Paused:
        m_statusLabel->setText(tr("Paused (High load or manual)"));
        m_pauseBtn->setText(tr("▶ Resume"));
        m_startBtn->setEnabled(false);
        m_pauseBtn->setEnabled(true);
        m_cancelBtn->setEnabled(true);
        setProfileControlsEnabled(false);
        break;
    case RsyncProcess::Finished:
        m_statusLabel->setText(tr("Completed"));
        m_startBtn->setEnabled(true);
        m_pauseBtn->setEnabled(false);
        m_cancelBtn->setEnabled(false);
        setProfileControlsEnabled(true);
        m_pauseBtn->setText(tr("⏸ Pause"));
        break;
    case RsyncProcess::Error:
        m_statusLabel->setText(tr("Error occurred"));
        m_startBtn->setEnabled(true);
        m_pauseBtn->setEnabled(false);
        m_cancelBtn->setEnabled(false);
        setProfileControlsEnabled(true);
        m_pauseBtn->setText(tr("⏸ Pause"));
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
    setProfileControlsEnabled(true);
    m_pauseBtn->setText(tr("⏸ Pause"));

    if (!success) {
        if (m_trayIcon) {
            m_trayIcon->showMessage(tr("Backup Error"), message, QSystemTrayIcon::Critical, 5000);
        }
        if (isVisible()) {
            QMessageBox::critical(this, tr("Error"), message);
        }
    }
}

void MainWindow::onLoadWarning(const QString &warningMsg)
{
    m_autoPauseIndicator->setText("🟡 " + warningMsg);
}

void MainWindow::onMetricsUpdated(double cpuPercent, double loadAvg)
{
    m_cpuBar->setValue(static_cast<int>(cpuPercent));
    m_loadAvgLabel->setText(QString::number(loadAvg, 'f', 2));
}

void MainWindow::onScheduledBackupTriggered(const BackupProfile &profile, bool isCatchUp)
{
    Q_UNUSED(profile);
    Q_UNUSED(isCatchUp);
}

void MainWindow::onScheduleDelayed(const QString &profileName, const QString &reason)
{
    Q_UNUSED(profileName);
    Q_UNUSED(reason);
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
    if (!AutostartManager::setAutostartEnabled(checked)) {
        QMessageBox::warning(this, tr("Error"), tr("Failed to update autostart setting."));
        if (m_autostartAction) {
            m_autostartAction->setChecked(!checked);
        }
    }
}

void MainWindow::onOpenHistory()
{
    HistoryDialog dialog(&m_historyManager, this);
    dialog.exec();
}

void MainWindow::setStartMinimized(bool minimized)
{
    if (minimized) {
        hide();
    } else {
        show();
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (!m_forceQuit && m_trayIcon->isVisible()) {
        event->ignore();
        this->hide();
    } else {
        if (m_backupController && (m_backupController->state() == RsyncProcess::Running || m_backupController->state() == RsyncProcess::Paused)) {
            m_backupController->cancelBackup();
        }
        event->accept();
    }
}
