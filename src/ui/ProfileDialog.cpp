#include "ProfileDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>

ProfileDialog::ProfileDialog(const BackupProfile &profile, QWidget *parent)
    : QDialog(parent), m_profile(profile)
{
    setupUi();
    retranslateUi();
    loadProfileToUi();
}

void ProfileDialog::setupUi()
{
    resize(520, 560);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QFormLayout *formLayout = new QFormLayout();

    m_nameLabel = new QLabel(this);
    m_nameEdit = new QLineEdit(this);
    formLayout->addRow(m_nameLabel, m_nameEdit);

    QHBoxLayout *srcLayout = new QHBoxLayout();
    m_srcLabel = new QLabel(this);
    m_sourceEdit = new QLineEdit(this);
    m_srcBtn = new QPushButton(this);
    srcLayout->addWidget(m_sourceEdit);
    srcLayout->addWidget(m_srcBtn);
    formLayout->addRow(m_srcLabel, srcLayout);

    QHBoxLayout *tgtLayout = new QHBoxLayout();
    m_tgtLabel = new QLabel(this);
    m_targetEdit = new QLineEdit(this);
    m_tgtBtn = new QPushButton(this);
    tgtLayout->addWidget(m_targetEdit);
    tgtLayout->addWidget(m_tgtBtn);
    formLayout->addRow(m_tgtLabel, tgtLayout);

    m_excludeLabel = new QLabel(this);
    m_excludeEdit = new QTextEdit(this);
    m_excludeEdit->setMaximumHeight(80);
    formLayout->addRow(m_excludeLabel, m_excludeEdit);

    mainLayout->addLayout(formLayout);

    connect(m_srcBtn, &QPushButton::clicked, this, &ProfileDialog::browseSource);
    connect(m_tgtBtn, &QPushButton::clicked, this, &ProfileDialog::browseTarget);

    // Schedule configuration group
    m_schedGroup = new QGroupBox(this);
    QVBoxLayout *schedLayout = new QVBoxLayout(m_schedGroup);

    m_scheduleCheck = new QCheckBox(m_schedGroup);
    schedLayout->addWidget(m_scheduleCheck);

    QHBoxLayout *timeLayout = new QHBoxLayout();
    m_freqLabel = new QLabel(m_schedGroup);
    m_scheduleTypeCombo = new QComboBox(m_schedGroup);
    m_scheduleTypeCombo->addItem("", "daily");
    m_scheduleTypeCombo->addItem("", "weekly");

    m_scheduleTimeEdit = new QTimeEdit(QTime(3, 0), m_schedGroup);
    m_scheduleTimeEdit->setDisplayFormat("HH:mm");

    m_timeLabel = new QLabel(m_schedGroup);

    timeLayout->addWidget(m_freqLabel);
    timeLayout->addWidget(m_scheduleTypeCombo);
    timeLayout->addWidget(m_timeLabel);
    timeLayout->addWidget(m_scheduleTimeEdit);
    timeLayout->addStretch();
    schedLayout->addLayout(timeLayout);

    // Day selection
    QHBoxLayout *daysLayout = new QHBoxLayout();
    m_daysLabel = new QLabel(m_schedGroup);
    daysLayout->addWidget(m_daysLabel);
    for (int i = 0; i < 7; ++i) {
        QCheckBox *cb = new QCheckBox(m_schedGroup);
        m_dayCheckBoxes.append(cb);
        daysLayout->addWidget(cb);
    }
    schedLayout->addLayout(daysLayout);

    mainLayout->addWidget(m_schedGroup);

    connect(m_scheduleCheck, &QCheckBox::toggled, this, &ProfileDialog::onScheduleToggled);

    // Smart load throttling settings group
    m_loadGroup = new QGroupBox(this);
    QFormLayout *loadLayout = new QFormLayout(m_loadGroup);

    m_autoPauseCheck = new QCheckBox(m_loadGroup);
    m_autoPauseCheck->setChecked(true);
    loadLayout->addRow(m_autoPauseCheck);

    m_cpuLabel = new QLabel(m_loadGroup);
    m_cpuSpin = new QDoubleSpinBox(m_loadGroup);
    m_cpuSpin->setRange(10.0, 100.0);
    m_cpuSpin->setSuffix(" %");
    m_cpuSpin->setValue(80.0);
    loadLayout->addRow(m_cpuLabel, m_cpuSpin);

    m_loadAvgLabel = new QLabel(m_loadGroup);
    m_loadAvgSpin = new QDoubleSpinBox(m_loadGroup);
    m_loadAvgSpin->setRange(0.5, 64.0);
    m_loadAvgSpin->setSingleStep(0.5);
    m_loadAvgSpin->setValue(4.0);
    loadLayout->addRow(m_loadAvgLabel, m_loadAvgSpin);

    mainLayout->addWidget(m_loadGroup);

    // Save & Cancel buttons
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    m_saveBtn = new QPushButton(this);
    m_cancelBtn = new QPushButton(this);
    m_saveBtn->setDefault(true);

    btnLayout->addWidget(m_saveBtn);
    btnLayout->addWidget(m_cancelBtn);
    mainLayout->addLayout(btnLayout);

    connect(m_saveBtn, &QPushButton::clicked, this, &ProfileDialog::onSaveClicked);
    connect(m_cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
}

void ProfileDialog::retranslateUi()
{
    setWindowTitle(tr("Profile Settings - GXBackup"));
    m_nameLabel->setText(tr("Profile Name:"));
    m_srcLabel->setText(tr("Backup Source:"));
    m_srcBtn->setText(tr("Browse..."));
    m_tgtLabel->setText(tr("Backup Target:"));
    m_tgtBtn->setText(tr("Browse..."));
    m_excludeLabel->setText(tr("Exclude Patterns:"));
    m_excludeEdit->setPlaceholderText(tr("Exclude patterns (1 per line)\nExample:\n.cache\nDownloads\n*.tmp"));

    m_schedGroup->setTitle(tr("📅 Automatic Schedule Execution"));
    m_scheduleCheck->setText(tr("Enable automatic scheduled backup"));
    m_freqLabel->setText(tr("Frequency:"));
    m_scheduleTypeCombo->setItemText(0, tr("Daily"));
    m_scheduleTypeCombo->setItemText(1, tr("Weekly"));
    m_timeLabel->setText(tr("Execution Time:"));
    m_daysLabel->setText(tr("Days:"));

    QStringList dayNames = {tr("Mon"), tr("Tue"), tr("Wed"), tr("Thu"), tr("Fri"), tr("Sat"), tr("Sun")};
    for (int i = 0; i < 7 && i < m_dayCheckBoxes.size(); ++i) {
        m_dayCheckBoxes[i]->setText(dayNames[i]);
    }

    m_loadGroup->setTitle(tr("⚡ Smart Load Throttling (Auto-Pause)"));
    m_autoPauseCheck->setText(tr("Automatically pause backup on high load"));
    m_cpuLabel->setText(tr("CPU threshold for pause:"));
    m_loadAvgLabel->setText(tr("Load average threshold for pause:"));

    m_saveBtn->setText(tr("Save"));
    m_cancelBtn->setText(tr("Cancel"));
}

void ProfileDialog::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QDialog::changeEvent(event);
}

void ProfileDialog::onScheduleToggled(bool enabled)
{
    m_scheduleTypeCombo->setEnabled(enabled);
    m_scheduleTimeEdit->setEnabled(enabled);
    for (auto cb : m_dayCheckBoxes) {
        cb->setEnabled(enabled);
    }
}

void ProfileDialog::loadProfileToUi()
{
    m_nameEdit->setText(m_profile.name);
    m_sourceEdit->setText(m_profile.sourceDir);
    m_targetEdit->setText(m_profile.targetDir);
    m_excludeEdit->setPlainText(m_profile.excludePatterns.join('\n'));

    m_scheduleCheck->setChecked(m_profile.scheduleEnabled);
    int typeIdx = m_scheduleTypeCombo->findData(m_profile.scheduleType);
    if (typeIdx >= 0) m_scheduleTypeCombo->setCurrentIndex(typeIdx);

    QTime t = QTime::fromString(m_profile.scheduleTime, "HH:mm");
    if (t.isValid()) m_scheduleTimeEdit->setTime(t);

    for (int i = 0; i < 7; ++i) {
        int dayNum = i + 1; // 1=Mon...7=Sun
        m_dayCheckBoxes[i]->setChecked(m_profile.scheduleDays.contains(dayNum));
    }
    onScheduleToggled(m_profile.scheduleEnabled);

    m_autoPauseCheck->setChecked(m_profile.autoPauseOnHighLoad);
    m_cpuSpin->setValue(m_profile.cpuThreshold);
    m_loadAvgSpin->setValue(m_profile.loadAvgThreshold);
}

void ProfileDialog::browseSource()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select Backup Source Directory"), m_sourceEdit->text());
    if (!dir.isEmpty()) {
        m_sourceEdit->setText(dir);
    }
}

void ProfileDialog::browseTarget()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select Backup Target Directory"), m_targetEdit->text());
    if (!dir.isEmpty()) {
        m_targetEdit->setText(dir);
    }
}

void ProfileDialog::applyUserDataPreset()
{
    QString target = m_targetEdit->text();
    m_profile = ProfileManager::createUserDataPreset(target.isEmpty() ? "/media/backup/gxbackup_home" : target);
    loadProfileToUi();
}

void ProfileDialog::applySystemPreset()
{
    QString target = m_targetEdit->text();
    m_profile = ProfileManager::createSystemPreset(target.isEmpty() ? "/media/backup/gxbackup_system" : target);
    loadProfileToUi();
}

void ProfileDialog::onSaveClicked()
{
    if (m_nameEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, tr("Input Error"), tr("Please enter a profile name."));
        return;
    }
    if (m_sourceEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, tr("Input Error"), tr("Please specify backup source."));
        return;
    }
    if (m_targetEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, tr("Input Error"), tr("Please specify backup target."));
        return;
    }

    m_profile.name = m_nameEdit->text().trimmed();
    m_profile.sourceDir = m_sourceEdit->text().trimmed();
    m_profile.targetDir = m_targetEdit->text().trimmed();
    
    QStringList excludes = m_excludeEdit->toPlainText().split('\n', Qt::SkipEmptyParts);
    m_profile.excludePatterns.clear();
    for (const QString &ex : excludes) {
        if (!ex.trimmed().isEmpty()) {
            m_profile.excludePatterns.append(ex.trimmed());
        }
    }

    m_profile.scheduleEnabled = m_scheduleCheck->isChecked();
    m_profile.scheduleType = m_scheduleTypeCombo->currentData().toString();
    m_profile.scheduleTime = m_scheduleTimeEdit->time().toString("HH:mm");

    m_profile.scheduleDays.clear();
    for (int i = 0; i < 7; ++i) {
        if (m_dayCheckBoxes[i]->isChecked()) {
            m_profile.scheduleDays.append(i + 1); // 1=Mon...7=Sun
        }
    }

    m_profile.autoPauseOnHighLoad = m_autoPauseCheck->isChecked();
    m_profile.cpuThreshold = m_cpuSpin->value();
    m_profile.loadAvgThreshold = m_loadAvgSpin->value();

    accept();
}

BackupProfile ProfileDialog::profile() const
{
    return m_profile;
}
