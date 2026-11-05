# ankaros compositor'ı: Qt Wayland Compositor ile yazılmış Wayland sunucusu ve masaüstü.
# Yalnızca Linux'ta derlenir (bkz. docs/kararlar/0006-qt-wayland-compositor-ve-eglfs.md).
cmake_minimum_required(VERSION 3.21)
project(ankaros-compositor VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 6.9 REQUIRED COMPONENTS Quick WaylandCompositor GuiPrivate)
# Klavye düzeninin compositor'ın kendi arayüzüne anında uygulanması (keyboardlayout.cpp)
find_package(PkgConfig REQUIRED)
pkg_check_modules(XKBCOMMON REQUIRED IMPORTED_TARGET xkbcommon)
# Qt'de olmayan protokollerin tanımları (docs/wayland-protokolleri.md)
pkg_get_variable(WAYLAND_PROTOCOLS_DIR wayland-protocols pkgdatadir)
if(NOT WAYLAND_PROTOCOLS_DIR)
    message(FATAL_ERROR "wayland-protocols bulunamadı")
endif()
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
        cursorshape.h
        cursorshape.cpp
        globalkeys.h
        globalkeys.cpp
        keyboardlayout.h
        keyboardlayout.cpp
        primaryselection.h
        primaryselection.cpp
        xdgactivation.h
        xdgactivation.cpp
)

# cursor-shape çizim tableti arayüzüne başvurur, tablet tanımı bu yüzden gerekli
qt6_generate_wayland_protocol_server_sources(ankaros-compositor
    FILES
        ${WAYLAND_PROTOCOLS_DIR}/staging/cursor-shape/cursor-shape-v1.xml
        ${WAYLAND_PROTOCOLS_DIR}/staging/xdg-activation/xdg-activation-v1.xml
        ${WAYLAND_PROTOCOLS_DIR}/unstable/primary-selection/primary-selection-unstable-v1.xml
        ${WAYLAND_PROTOCOLS_DIR}/unstable/tablet/tablet-unstable-v2.xml
)
target_include_directories(ankaros-compositor PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(ankaros-compositor PRIVATE Qt6::Quick Qt6::WaylandCompositor Qt6::GuiPrivate
    PkgConfig::XKBCOMMON)

install(TARGETS ankaros-compositor RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
