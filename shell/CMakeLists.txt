# ankaros masaüstü kabuğu.
# Ankaros.Shell QML modülü hem Mac önizlemesinde hem de (Faz 1'de) compositor'da kullanılır.
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
option(ANKAROS_SHELL_SHARED "Ankaros.Shell'i paylaşılan bir QML modülü olarak derle ve kur" OFF)

add_subdirectory(qml/Ankaros/Shell)

option(ANKAROS_SHELL_PREVIEW "Kabuğu sıradan bir pencerede gösteren önizleme uygulamasını derle" ON)
if(ANKAROS_SHELL_PREVIEW)
    add_subdirectory(preview)
endif()
