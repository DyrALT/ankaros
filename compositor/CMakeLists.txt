# ankaros compositor'ı: Qt Wayland Compositor ile yazılmış Wayland sunucusu ve masaüstü.
# Yalnızca Linux'ta derlenir (bkz. docs/kararlar/0006-qt-wayland-compositor-ve-eglfs.md).
cmake_minimum_required(VERSION 3.21)
project(ankaros-compositor VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 6.9 REQUIRED COMPONENTS Quick WaylandCompositor GuiPrivate)
# Klavye düzeninin compositor'ın kendi arayüzüne anında uygulanması (keyboardlayout.cpp)
find_package(PkgConfig REQUIRED)
pkg_check_modules(XKBCOMMON REQUIRED IMPORTED_TARGET xkbcommon)
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
        applauncher.h
        applauncher.cpp
        globalkeys.h
        globalkeys.cpp
        keyboardlayout.h
        keyboardlayout.cpp
)

target_link_libraries(ankaros-compositor PRIVATE Qt6::Quick Qt6::WaylandCompositor Qt6::GuiPrivate
    PkgConfig::XKBCOMMON)

install(TARGETS ankaros-compositor RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
