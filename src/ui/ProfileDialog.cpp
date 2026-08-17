#include "ProfileDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QFileDialog>
#include <QGroupBox>
#include <QLabel>
#include <QMessageBox>
#include <QTime>

ProfileDialog::ProfileDialog(const BackupProfile &profile, QWidget *parent)
    : QDialog(parent), m_profile(profile)
{
    setupUi();
    loadProfileToUi();
}

void ProfileDialog::setupUi()
{
    setWindowTitle("プロファイル設定 - GXBackup");
    resize(580, 620);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // プリセットボタン
    QHBoxLayout *presetLayout = new QHBoxLayout();
    QPushButton *userPresetBtn = new QPushButton("👤 ユーザーデータ プリセット設定", this);
    QPushButton *sysPresetBtn = new QPushButton("💻 システム全体 プリセット設定", this);
    presetLayout->addWidget(userPresetBtn);
    presetLayout->addWidget(sysPresetBtn);
    mainLayout->addLayout(presetLayout);

    connect(userPresetBtn, &QPushButton::clicked, this, &ProfileDialog::applyUserDataPreset);
    connect(sysPresetBtn, &QPushButton::clicked, this, &ProfileDialog::applySystemPreset);

    // 基本設定フォーム
    QFormLayout *formLayout = new QFormLayout();

    m_nameEdit = new QLineEdit(this);
    formLayout->addRow("プロファイル名:", m_nameEdit);

    QHBoxLayout *srcLayout = new QHBoxLayout();
    m_sourceEdit = new QLineEdit(this);
    QPushButton *srcBtn = new QPushButton("参照...", this);
    srcLayout->addWidget(m_sourceEdit);
    srcLayout->addWidget(srcBtn);
    formLayout->addRow("バックアップ元 (Source):", srcLayout);

    QHBoxLayout *tgtLayout = new QHBoxLayout();
    m_targetEdit = new QLineEdit(this);
    QPushButton *tgtBtn = new QPushButton("参照...", this);
    tgtLayout->addWidget(m_targetEdit);
    tgtLayout->addWidget(tgtBtn);
    formLayout->addRow("バックアップ先 (Target):", tgtLayout);

    m_excludeEdit = new QTextEdit(this);
    m_excludeEdit->setPlaceholderText("除外パターン (1行に1つ)\n例:\n.cache\nDownloads\n*.tmp");
    m_excludeEdit->setMaximumHeight(80);
    formLayout->addRow("除外パターン:", m_excludeEdit);

    mainLayout->addLayout(formLayout);

    connect(srcBtn, &QPushButton::clicked, this, &ProfileDialog::browseSource);
    connect(tgtBtn, &QPushButton::clicked, this, &ProfileDialog::browseTarget);

    // スケジュール設定グループ
    QGroupBox *schedGroup = new QGroupBox("📅 自動スケジュール実行", this);
    QVBoxLayout *schedLayout = new QVBoxLayout(schedGroup);

    m_scheduleCheck = new QCheckBox("自動スケジュールバックアップを有効にする", schedGroup);
    schedLayout->addWidget(m_scheduleCheck);

    QHBoxLayout *timeLayout = new QHBoxLayout();
    m_scheduleTypeCombo = new QComboBox(schedGroup);
    m_scheduleTypeCombo->addItem("毎日 (Daily)", "daily");
    m_scheduleTypeCombo->addItem("毎週 (Weekly)", "weekly");

    m_scheduleTimeEdit = new QTimeEdit(QTime(3, 0), schedGroup);
    m_scheduleTimeEdit->setDisplayFormat("HH:mm");

    timeLayout->addWidget(new QLabel("頻度:"));
    timeLayout->addWidget(m_scheduleTypeCombo);
    timeLayout->addWidget(new QLabel("実行時刻:"));
    timeLayout->addWidget(m_scheduleTimeEdit);
    timeLayout->addStretch();
    schedLayout->addLayout(timeLayout);

    // 曜日選択
    QHBoxLayout *daysLayout = new QHBoxLayout();
    daysLayout->addWidget(new QLabel("実行曜日:"));
    QStringList dayNames = {"月", "火", "水", "木", "金", "土", "日"};
    for (int i = 0; i < 7; ++i) {
        QCheckBox *cb = new QCheckBox(dayNames[i], schedGroup);
        m_dayCheckBoxes.append(cb);
        daysLayout->addWidget(cb);
    }
    schedLayout->addLayout(daysLayout);

    mainLayout->addWidget(schedGroup);

    connect(m_scheduleCheck, &QCheckBox::toggled, this, &ProfileDialog::onScheduleToggled);

    // スマート負荷制限設定グループ
    QGroupBox *loadGroup = new QGroupBox("⚡ スマート負荷制限 (Auto-Pause/Throttling)", this);
    QFormLayout *loadLayout = new QFormLayout(loadGroup);

    m_autoPauseCheck = new QCheckBox("高負荷時にバックアップ処理を自動一時停止する", loadGroup);
    m_autoPauseCheck->setChecked(true);
    loadLayout->addRow(m_autoPauseCheck);

    m_cpuSpin = new QDoubleSpinBox(loadGroup);
    m_cpuSpin->setRange(10.0, 100.0);
    m_cpuSpin->setSuffix(" %");
    m_cpuSpin->setValue(80.0);
    loadLayout->addRow("一時停止するCPU使用率閾値:", m_cpuSpin);

    m_loadAvgSpin = new QDoubleSpinBox(loadGroup);
    m_loadAvgSpin->setRange(0.5, 64.0);
    m_loadAvgSpin->setSingleStep(0.5);
    m_loadAvgSpin->setValue(4.0);
    loadLayout->addRow("一時停止するLoad Average閾値:", m_loadAvgSpin);

    mainLayout->addWidget(loadGroup);

    // 保存・キャンセルボタン
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    QPushButton *saveBtn = new QPushButton("保存", this);
    QPushButton *cancelBtn = new QPushButton("キャンセル", this);
    saveBtn->setDefault(true);

    btnLayout->addWidget(saveBtn);
    btnLayout->addWidget(cancelBtn);
    mainLayout->addLayout(btnLayout);

    connect(saveBtn, &QPushButton::clicked, this, &ProfileDialog::onSaveClicked);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
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
    QString dir = QFileDialog::getExistingDirectory(this, "バックアップ元ディレクトリを選択", m_sourceEdit->text());
    if (!dir.isEmpty()) {
        m_sourceEdit->setText(dir);
    }
}

void ProfileDialog::browseTarget()
{
    QString dir = QFileDialog::getExistingDirectory(this, "バックアップ先ディレクトリを選択", m_targetEdit->text());
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
        QMessageBox::warning(this, "入力エラー", "プロファイル名を入力してください。");
        return;
    }
    if (m_sourceEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "入力エラー", "バックアップ元を指定してください。");
        return;
    }
    if (m_targetEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "入力エラー", "バックアップ先を指定してください。");
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
