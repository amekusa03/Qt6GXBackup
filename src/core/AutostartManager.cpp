#include "AutostartManager.h"
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QTextStream>

QString AutostartManager::getAutostartFilePath()
{
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    QDir autostartDir(configDir + "/autostart");
    if (!autostartDir.exists()) {
        autostartDir.mkpath(".");
    }
    return autostartDir.filePath("gxbackup.desktop");
}

bool AutostartManager::isAutostartEnabled()
{
    return QFile::exists(getAutostartFilePath());
}

bool AutostartManager::setAutostartEnabled(bool enable)
{
    QString filePath = getAutostartFilePath();

    if (!enable) {
        if (QFile::exists(filePath)) {
            return QFile::remove(filePath);
        }
        return true;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QString execPath = QCoreApplication::applicationFilePath();

    QTextStream out(&file);
    out << "[Desktop Entry]\n";
    out << "Type=Application\n";
    out << "Name=GXBackup Autostart\n";
    out << "Comment=Linux Smart Backup Tool System Tray Autostart\n";
    out << "Exec=" << execPath << " --autostart\n";
    out << "Icon=system-file-manager\n";
    out << "Terminal=false\n";
    out << "Categories=System;\n";
    out << "X-GNOME-Autostart-enabled=true\n";

    return true;
}
