#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QComboBox>
#include <QPushButton>
#include <QProgressBar>
#include <QLabel>
#include <QTextEdit>
#include <QGroupBox>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QActionGroup>
#include <QCloseEvent>
#include <QTimer>
#include <QStringList>
#include <QEvent>
#include "ProfileManager.h"
#include "HistoryManager.h"
#include "BackupController.h"
#include "ScheduleManager.h"
#include "LogViewer.h"
#include "HistoryDialog.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;

    void setStartMinimized(bool minimized);

protected:
    void closeEvent(QCloseEvent *event) override;
    void changeEvent(QEvent *event) override;

private slots:
    void onNewProfile();
    void onEditProfile();
    void onDeleteProfile();
    void onProfileSelected(int index);

    void onStartBackup();
    void onPauseResumeBackup();
    void onCancelBackup();
    void onOpenHistory();

    void onStateChanged(RsyncProcess::State state);
    void onProgressUpdated(int percent, QString speed, QString transferred, QString remainingTime);
    void onLogMessage(const QString &msg);
    void onBackupFinished(bool success, const QString &message);
    void onLoadWarning(const QString &warningMsg);
    void onMetricsUpdated(double cpuPercent, double loadAvg);
    void flushLogBuffer();

    // Schedule & System Tray related
    void onScheduledBackupTriggered(const BackupProfile &profile, bool isCatchUp);
    void onScheduleDelayed(const QString &profileName, const QString &reason);
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void onToggleAutostart(bool checked);

    // Language setting
    void onLanguageSelected(QAction *action);

private:
    ProfileManager m_profileManager;
    HistoryManager m_historyManager;
    BackupController *m_backupController = nullptr;
    ScheduleManager *m_scheduleManager = nullptr;
    LogViewer *m_logViewer = nullptr;

    // Menu Bar & Settings
    QMenu *m_settingsMenu = nullptr;
    QMenu *m_languageMenu = nullptr;
    QAction *m_englishAction = nullptr;
    QAction *m_japaneseAction = nullptr;
    QActionGroup *m_langActionGroup = nullptr;

    // System Tray
    QSystemTrayIcon *m_trayIcon = nullptr;
    QMenu *m_trayMenu = nullptr;
    QAction *m_trayOpenAction = nullptr;
    QAction *m_trayStartAction = nullptr;
    QAction *m_trayHistoryAction = nullptr;
    QAction *m_autostartAction = nullptr;
    QAction *m_trayQuitAction = nullptr;
    bool m_forceQuit = false;

    // UI Component Groups & Labels
    QGroupBox *m_profileGroup = nullptr;
    QComboBox *m_profileCombo = nullptr;
    QPushButton *m_newBtn = nullptr;
    QPushButton *m_editBtn = nullptr;
    QPushButton *m_deleteBtn = nullptr;
    QPushButton *m_historyBtn = nullptr;

    QGroupBox *m_statusGroup = nullptr;
    QLabel *m_statusLabel = nullptr;
    QProgressBar *m_progressBar = nullptr;
    QLabel *m_speedTitleLabel = nullptr;
    QLabel *m_speedLabel = nullptr;
    QLabel *m_transferredTitleLabel = nullptr;
    QLabel *m_transferredLabel = nullptr;
    QLabel *m_remainingTitleLabel = nullptr;
    QLabel *m_remainingLabel = nullptr;

    QGroupBox *m_monitorGroup = nullptr;
    QLabel *m_cpuTitleLabel = nullptr;
    QProgressBar *m_cpuBar = nullptr;
    QLabel *m_loadAvgTitleLabel = nullptr;
    QLabel *m_loadAvgLabel = nullptr;
    QLabel *m_autoPauseIndicator = nullptr;

    QPushButton *m_startBtn = nullptr;
    QPushButton *m_pauseBtn = nullptr;
    QPushButton *m_cancelBtn = nullptr;

    QGroupBox *m_logGroup = nullptr;
    QTextEdit *m_logEdit = nullptr;
    QStringList m_logBuffer;
    QTimer *m_logFlushTimer = nullptr;

    void setupUi();
    void setupMenuBar();
    void setupTrayIcon();
    void applyStyleSheet();
    void updateProfileCombo();
    void retranslateUi();
    void setProfileControlsEnabled(bool enabled);
    BackupProfile currentSelectedProfile() const;
};

#endif // MAINWINDOW_H
