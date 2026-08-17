#ifndef LOGVIEWER_H
#define LOGVIEWER_H

#include <QDialog>
#include <QTextEdit>
#include <QPushButton>

class LogViewer : public QDialog
{
    Q_OBJECT
public:
    explicit LogViewer(QWidget *parent = nullptr);
    ~LogViewer() override = default;

    void appendLog(const QString &line);
    void appendLogs(const QStringList &lines);
    void clearLog();

private:
    QTextEdit *m_textEdit;
    QPushButton *m_clearBtn;
    QPushButton *m_closeBtn;

    void setupUi();
};

#endif // LOGVIEWER_H
