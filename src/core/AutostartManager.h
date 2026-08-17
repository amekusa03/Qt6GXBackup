#ifndef AUTOSTARTMANAGER_H
#define AUTOSTARTMANAGER_H

#include <QString>

class AutostartManager
{
public:
    static bool isAutostartEnabled();
    static bool setAutostartEnabled(bool enable);

private:
    static QString getAutostartFilePath();
};

#endif // AUTOSTARTMANAGER_H
