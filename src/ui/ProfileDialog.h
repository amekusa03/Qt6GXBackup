#ifndef PROFILEDIALOG_H
#define PROFILEDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QTextEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QTimeEdit>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QList>
#include <QEvent>
#include "ProfileManager.h"

class ProfileDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProfileDialog(const BackupProfile &profile = BackupProfile(), QWidget *parent = nullptr);
    ~ProfileDialog() override = default;

    BackupProfile profile() const;

protected:
    void changeEvent(QEvent *event) override;

private slots:
    void browseSource();
    void browseTarget();
    void applyUserDataPreset();
    void applySystemPreset();
    void onScheduleToggled(bool enabled);
    void onSaveClicked();

private:
    BackupProfile m_profile;

    QLabel *m_nameLabel;
    QLineEdit *m_nameEdit;

    QLabel *m_srcLabel;
    QLineEdit *m_sourceEdit;
    QPushButton *m_srcBtn;

    QLabel *m_tgtLabel;
    QLineEdit *m_targetEdit;
    QPushButton *m_tgtBtn;

    QLabel *m_excludeLabel;
    QTextEdit *m_excludeEdit;

    QGroupBox *m_schedGroup;
    QCheckBox *m_scheduleCheck;
    QLabel *m_freqLabel;
    QComboBox *m_scheduleTypeCombo;
    QLabel *m_timeLabel;
    QTimeEdit *m_scheduleTimeEdit;
    QLabel *m_daysLabel;
    QList<QCheckBox*> m_dayCheckBoxes;

    QGroupBox *m_loadGroup;
    QCheckBox *m_autoPauseCheck;
    QLabel *m_cpuLabel;
    QDoubleSpinBox *m_cpuSpin;
    QLabel *m_loadAvgLabel;
    QDoubleSpinBox *m_loadAvgSpin;

    QPushButton *m_saveBtn;
    QPushButton *m_cancelBtn;

    void setupUi();
    void retranslateUi();
    void loadProfileToUi();
};

#endif // PROFILEDIALOG_H
