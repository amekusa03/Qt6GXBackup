#include "LanguageManager.h"
#include <QCoreApplication>
#include <QSettings>
#include <QDebug>

LanguageManager& LanguageManager::instance()
{
    static LanguageManager mgr;
    return mgr;
}

LanguageManager::LanguageManager(QObject *parent)
    : QObject(parent), m_currentLang("en")
{
}

QString LanguageManager::currentLanguage() const
{
    return m_currentLang;
}

void LanguageManager::initLanguage()
{
    QSettings settings;
    QString lang = settings.value("Language", "en").toString();
    setLanguage(lang);
}

void LanguageManager::setLanguage(const QString &langCode)
{
    QCoreApplication::removeTranslator(&m_translator);

    if (langCode == "ja") {
        if (m_translator.load(":/translations/gxbackup_ja.qm")) {
            QCoreApplication::installTranslator(&m_translator);
        } else if (m_translator.load("gxbackup_ja.qm", ":/translations")) {
            QCoreApplication::installTranslator(&m_translator);
        } else {
            qWarning() << "Failed to load Japanese translation file";
        }
    }

    m_currentLang = langCode;

    QSettings settings;
    settings.setValue("Language", langCode);

    emit languageChanged(langCode);
}
