#ifndef HISTORYDIALOG_H
#define HISTORYDIALOG_H

#include <QDialog>
#include <QTableWidget>
#include <QPushButton>
#include "HistoryManager.h"

class HistoryDialog : public QDialog
{
    Q_OBJECT
public:
    explicit HistoryDialog(HistoryManager *historyManager, QWidget *parent = nullptr);
    ~HistoryDialog() override = default;

    void refreshHistory();

private:
    HistoryManager *m_historyManager;
    QTableWidget *m_tableWidget;
    QPushButton *m_refreshBtn;
    QPushButton *m_closeBtn;

    void setupUi();
};

#endif // HISTORYDIALOG_H
