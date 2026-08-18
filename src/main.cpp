#include <QApplication>
#include <QIcon>
#include <QCommandLineParser>
#include "MainWindow.h"
#include "LanguageManager.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);
    
    app.setApplicationName("GXBackup");
    app.setApplicationVersion("0.1.0");
    app.setOrganizationName("GXBackupTeam");
    app.setDesktopFileName("gxbackup"); 
    app.setWindowIcon(QIcon(":/images/icon.png"));

    LanguageManager::instance().initLanguage();

    QCommandLineParser parser;
    parser.setApplicationDescription(QCoreApplication::translate("main", "Linux Smart Backup Tool"));
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption autostartOption(QStringList() << "a" << "autostart", QCoreApplication::translate("main", "Start application in system tray minimized for autostart."));
    parser.addOption(autostartOption);

    QCommandLineOption minimizedOption(QStringList() << "m" << "minimized", QCoreApplication::translate("main", "Start application minimized to system tray."));
    parser.addOption(minimizedOption);

    parser.process(app);

    bool startMinimized = parser.isSet(autostartOption) || parser.isSet(minimizedOption);

    MainWindow window;
    window.setStartMinimized(startMinimized);

    return app.exec();
}
