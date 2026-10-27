qt_add_library(ankaros_shell STATIC)

set_source_files_properties(Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

qt_add_qml_module(ankaros_shell
    URI Ankaros.Shell
    VERSION 1.0
    QML_FILES
        Theme.qml
        Desktop.qml
        Wallpaper.qml
        Panel.qml
        PanelButton.qml
        LauncherIcon.qml
        Clock.qml
        Launcher.qml
        AppTile.qml
)

target_link_libraries(ankaros_shell PRIVATE Qt6::Quick)
