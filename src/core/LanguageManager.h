#ifndef LANGUAGEMANAGER_H
#define LANGUAGEMANAGER_H

#include <QObject>
#include <QTranslator>
#include <QString>

class LanguageManager : public QObject
{
    Q_OBJECT

public:
    static LanguageManager& instance();

    QString currentLanguage() const;
    void setLanguage(const QString &langCode);
    void initLanguage();

signals:
    void languageChanged(const QString &langCode);

private:
    explicit LanguageManager(QObject *parent = nullptr);
    ~LanguageManager() override = default;

    Q_DISABLE_COPY(LanguageManager)

    QTranslator m_translator;
    QString m_currentLang;
};

#endif // LANGUAGEMANAGER_H
