# ankaros masaüstü kabuğu ve ortak QML modülleri (karar 0013):
#   Ankaros.UI      tema, simgeler, ortak bileşenler (qml/Ankaros/UI)
#   Ankaros.System  sistem servislerinin istemcileri: oturum, ağ (qml/Ankaros/System)
#   Ankaros.Shell   masaüstü: alt çubuk, başlat paneli, giriş ve kilit ekranı (qml/Ankaros/Shell)
# Modüller hem Mac önizlemesinde hem de compositor'da kullanılır.
cmake_minimum_required(VERSION 3.21)
project(ankaros-shell VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 6.9: RectangularShadow (QtQuick.Effects)
find_package(Qt6 6.9 REQUIRED COMPONENTS Quick)
include(GNUInstallDirs)
qt_standard_project_setup(REQUIRES 6.5)

# Tüm QML modülleri tek bir klasörde toplanır, qmllint ve qmlls modülleri buradan bulur.
set(QT_QML_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/qml)

# Mac önizlemesinde modül önizleme uygulamasına statik olarak gömülür. ankaros imajında ise
# compositor'ın da kullanabilmesi için paylaşılan bir QML modülü olarak Qt'nin QML klasörüne kurulur.
option(ANKAROS_SHELL_SHARED "QML modüllerini paylaşılan modül olarak derle ve kur" OFF)

# Paylaşılan derlemede modülü Qt'nin QML klasörüne (Buildroot'ta /usr/qml) kurar. Modülü kullanan
# uygulamalar (compositor, Terminal, Ayarlar) onu buradan bulur.
function(ankaros_install_qml_module target path)
    if(NOT ANKAROS_SHELL_SHARED)
        return()
    endif()
    set(module_dir ${QT6_INSTALL_QML}/${path})
    install(TARGETS ${target} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
    install(TARGETS ${target}plugin LIBRARY DESTINATION ${module_dir})
    string(REPLACE "/" "_" types ${path})
    string(TOLOWER ${target} lower)
    install(FILES
        ${QT_QML_OUTPUT_DIRECTORY}/${path}/qmldir
        ${QT_QML_OUTPUT_DIRECTORY}/${path}/${lower}.qmltypes
        DESTINATION ${module_dir}
    )
endfunction()

add_subdirectory(qml/Ankaros/UI)
add_subdirectory(qml/Ankaros/System)
add_subdirectory(qml/Ankaros/Shell)

# Uygulamalar (karar 0013)
add_subdirectory(apps/settings)

option(ANKAROS_SHELL_PREVIEW "Kabuğu sıradan bir pencerede gösteren önizleme uygulamasını derle" ON)
if(ANKAROS_SHELL_PREVIEW)
    add_subdirectory(preview)
endif()
