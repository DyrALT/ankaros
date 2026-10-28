# Ankaros.UI: tema, yazı tipi, simgeler ve ortak bileşenler (karar 0013). Kabuk ve bütün uygulamalar
# bunu kullanır, böylece hepsi aynı tasarım dilinde olur.
if(ANKAROS_SHELL_SHARED)
    qt_add_library(ankaros_ui SHARED)
else()
    qt_add_library(ankaros_ui STATIC)
endif()

set_source_files_properties(Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

qt_add_qml_module(ankaros_ui
    URI Ankaros.UI
    VERSION 1.0
    QML_FILES
        Theme.qml
        IconData.js
        Label.qml
        Icon.qml
        AppIcon.qml
        Avatar.qml
        Bubble.qml
        GlassPanel.qml
        HoverButton.qml
        InfoTip.qml
        InputField.qml
        PushButton.qml
        SectionTitle.qml
        SegmentedControl.qml
        SettingLabel.qml
        ToggleSwitch.qml
        WindowTitleBar.qml
        Wallpaper.qml
    SOURCES
        appearance.h
        appearance.cpp
    RESOURCES
        fonts/Inter-Regular.ttf
        fonts/Inter-Medium.ttf
        fonts/Inter-SemiBold.ttf
        fonts/Inter-Bold.ttf
        fonts/Inter-LICENSE.txt
)

target_link_libraries(ankaros_ui PRIVATE Qt6::Quick)
ankaros_install_qml_module(ankaros_ui Ankaros/UI)
