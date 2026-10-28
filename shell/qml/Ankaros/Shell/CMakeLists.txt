if(ANKAROS_SHELL_SHARED)
    qt_add_library(ankaros_shell SHARED)
else()
    qt_add_library(ankaros_shell STATIC)
endif()

qt_add_qml_module(ankaros_shell
    URI Ankaros.Shell
    VERSION 1.0
    DEPENDENCIES Ankaros.UI
    QML_FILES
        Wallpaper.qml
        Desktop.qml
        TaskBar.qml
        TaskItem.qml
        Clock.qml
        StartMenu.qml
        AppTile.qml
        RecommendedItem.qml
        WindowTitleBar.qml
        WindowChrome.qml
        AppearanceSettings.qml
        BigClock.qml
        ErrorPill.qml
        GlassIconButton.qml
        LockBackground.qml
        LockScreen.qml
        LoginScreen.qml
        NetworkSettings.qml
        PasswordField.qml
        PowerMenu.qml
        SettingsPage.qml
        SettingsPanel.qml
        UserSettings.qml
)

target_link_libraries(ankaros_shell PRIVATE Qt6::Quick ankaros_ui)

ankaros_install_qml_module(ankaros_shell Ankaros/Shell)
