#ifndef PROFILEDIALOG_H
#define PROFILEDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QTextEdit>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QTimeEdit>
#include <QComboBox>
#include <QPushButton>
#include <QList>
#include "ProfileManager.h"

class ProfileDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ProfileDialog(const BackupProfile &profile = BackupProfile(), QWidget *parent = nullptr);
    ~ProfileDialog() override = default;

    BackupProfile profile() const;

private slots:
    void browseSource();
    void browseTarget();
    void applyUserDataPreset();
    void applySystemPreset();
    void onSaveClicked();
    void onScheduleToggled(bool enabled);

private:
    BackupProfile m_profile;

    QLineEdit *m_nameEdit;
    QLineEdit *m_sourceEdit;
    QLineEdit *m_targetEdit;
    QTextEdit *m_excludeEdit;

    // スケジュール設定UI
    QCheckBox *m_scheduleCheck;
    QComboBox *m_scheduleTypeCombo;
    QTimeEdit *m_scheduleTimeEdit;
    QList<QCheckBox*> m_dayCheckBoxes; // 月(0)〜日(6)

    // スマート負荷制限設定UI
    QCheckBox *m_autoPauseCheck;
    QDoubleSpinBox *m_cpuSpin;
    QDoubleSpinBox *m_loadAvgSpin;

    void setupUi();
    void loadProfileToUi();
};

#endif // PROFILEDIALOG_H
