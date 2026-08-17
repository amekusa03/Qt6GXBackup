cmake_minimum_required(VERSION 3.16)

project(GXBackup VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui Widgets Concurrent)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets Concurrent)

set(SOURCES
    src/main.cpp
    src/core/SystemMonitor.cpp
    src/core/RsyncProcess.cpp
    src/core/ProfileManager.cpp
    src/core/HistoryManager.cpp
    src/core/BackupController.cpp
    src/core/ScheduleManager.cpp
    src/core/AutostartManager.cpp
    src/ui/MainWindow.cpp
    src/ui/ProfileDialog.cpp
    src/ui/HistoryDialog.cpp
    src/ui/LogViewer.cpp
)

set(HEADERS
    src/core/SystemMonitor.h
    src/core/RsyncProcess.h
    src/core/ProfileManager.h
    src/core/HistoryManager.h
    src/core/BackupController.h
    src/core/ScheduleManager.h
    src/core/AutostartManager.h
    src/ui/MainWindow.h
    src/ui/ProfileDialog.h
    src/ui/HistoryDialog.h
    src/ui/LogViewer.h
)

add_executable(GXBackup ${SOURCES} ${HEADERS})

target_link_libraries(GXBackup PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Concurrent
)

target_include_directories(GXBackup PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui
)
