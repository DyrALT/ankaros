qt_add_library(ankaros_shell STATIC)

set_source_files_properties(Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

qt_add_qml_module(ankaros_shell
    URI Ankaros.Shell
    VERSION 1.0
    QML_FILES
        Theme.qml
        IconData.js
        Label.qml
        Icon.qml
        AppIcon.qml
        GlassPanel.qml
        HoverButton.qml
        Wallpaper.qml
        Desktop.qml
        TaskBar.qml
        TaskItem.qml
        Clock.qml
        StartMenu.qml
        AppTile.qml
        RecommendedItem.qml
    RESOURCES
        fonts/Inter-Regular.ttf
        fonts/Inter-Medium.ttf
        fonts/Inter-SemiBold.ttf
        fonts/Inter-Bold.ttf
        fonts/Inter-LICENSE.txt
)

target_link_libraries(ankaros_shell PRIVATE Qt6::Quick)
