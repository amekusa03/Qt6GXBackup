#ifndef HISTORYDIALOG_H
#define HISTORYDIALOG_H

#include <QDialog>
#include <QTableWidget>
#include <QPushButton>
#include <QEvent>
#include "HistoryManager.h"

class HistoryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit HistoryDialog(HistoryManager *historyManager, QWidget *parent = nullptr);
    ~HistoryDialog() override = default;

public slots:
    void refreshHistory();

protected:
    void changeEvent(QEvent *event) override;

private:
    HistoryManager *m_historyManager = nullptr;
    QTableWidget *m_tableWidget = nullptr;
    QPushButton *m_refreshBtn = nullptr;
    QPushButton *m_closeBtn = nullptr;

    void setupUi();
    void retranslateUi();
};

#endif // HISTORYDIALOG_H
