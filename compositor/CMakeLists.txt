# ankaros compositor'ı: Qt Wayland Compositor ile yazılmış Wayland sunucusu ve masaüstü.
# Yalnızca Linux'ta derlenir (bkz. docs/kararlar/0006-qt-wayland-compositor-ve-eglfs.md).
cmake_minimum_required(VERSION 3.21)
project(ankaros-compositor VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 6.9 REQUIRED COMPONENTS Quick WaylandCompositor)
include(GNUInstallDirs)
qt_standard_project_setup(REQUIRES 6.9)

qt_add_executable(ankaros-compositor main.cpp)

qt_add_qml_module(ankaros-compositor
    URI Ankaros.Compositor
    VERSION 1.0
    QML_FILES
        Main.qml
        WaylandWindow.qml
    SOURCES
        appcatalog.h
        appcatalog.cpp
        applauncher.h
        applauncher.cpp
        session.h
        session.cpp
)

target_link_libraries(ankaros-compositor PRIVATE Qt6::Quick Qt6::WaylandCompositor)

install(TARGETS ankaros-compositor RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
