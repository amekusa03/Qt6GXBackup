#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QComboBox>
#include <QPushButton>
#include <QProgressBar>
#include <QLabel>
#include <QTextEdit>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QCloseEvent>
#include <QTimer>
#include <QStringList>
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

    // スケジュール＆トレイ関連
    void onScheduledBackupTriggered(const BackupProfile &profile, bool isCatchUp);
    void onScheduleDelayed(const QString &profileName, const QString &reason);
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void onToggleAutostart(bool checked);

private:
    ProfileManager m_profileManager;
    HistoryManager m_historyManager;
    BackupController *m_backupController = nullptr;
    ScheduleManager *m_scheduleManager = nullptr;
    LogViewer *m_logViewer = nullptr;

    // トレイ常駐
    QSystemTrayIcon *m_trayIcon = nullptr;
    QMenu *m_trayMenu = nullptr;
    QAction *m_autostartAction = nullptr;
    bool m_forceQuit = false;

    // UI要素
    QComboBox *m_profileCombo;
    QPushButton *m_newBtn;
    QPushButton *m_editBtn;
    QPushButton *m_deleteBtn;
    QPushButton *m_historyBtn;

    QLabel *m_statusLabel;
    QProgressBar *m_progressBar;
    QLabel *m_speedLabel;
    QLabel *m_transferredLabel;
    QLabel *m_remainingLabel;

    QPushButton *m_startBtn;
    QPushButton *m_pauseBtn;
    QPushButton *m_cancelBtn;

    // モニター要素
    QProgressBar *m_cpuBar;
    QLabel *m_loadAvgLabel;
    QLabel *m_autoPauseIndicator;

    QTextEdit *m_logEdit;
    QStringList m_logBuffer;
    QTimer *m_logFlushTimer = nullptr;

    void setupUi();
    void setupTrayIcon();
    void applyStyleSheet();
    void updateProfileCombo();
    BackupProfile currentSelectedProfile() const;
};

#endif // MAINWINDOW_H
