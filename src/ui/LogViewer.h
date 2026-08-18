#ifndef LOGVIEWER_H
#define LOGVIEWER_H

#include <QDialog>
#include <QTextEdit>
#include <QPushButton>
#include <QEvent>

class LogViewer : public QDialog
{
    Q_OBJECT

public:
    explicit LogViewer(QWidget *parent = nullptr);
    ~LogViewer() override = default;

    void appendLog(const QString &line);
    void appendLogs(const QStringList &lines);

public slots:
    void clearLog();

protected:
    void changeEvent(QEvent *event) override;

private:
    QTextEdit *m_textEdit;
    QPushButton *m_clearBtn;
    QPushButton *m_closeBtn;

    void setupUi();
    void retranslateUi();
};

#endif // LOGVIEWER_H
