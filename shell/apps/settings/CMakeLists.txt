# Ayarlar uygulaması (karar 0013). Ankaros.UI ve Ankaros.System modüllerini kullanır.
qt_add_executable(ankaros-settings MACOSX_BUNDLE main.cpp)

qt_add_qml_module(ankaros-settings
    URI Ankaros.Apps.Settings
    VERSION 1.0
    QML_FILES
        Main.qml
        SettingsPanel.qml
        SettingsPage.qml
        AppearanceSettings.qml
        NetworkSettings.qml
        UserSettings.qml
)

target_link_libraries(ankaros-settings PRIVATE Qt6::Quick)
if(NOT ANKAROS_SHELL_SHARED)
    target_link_libraries(ankaros-settings PRIVATE ankaros_uiplugin ankaros_systemplugin)
    target_compile_definitions(ankaros-settings PRIVATE ANKAROS_SHELL_STATIC)
endif()

install(TARGETS ankaros-settings
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
