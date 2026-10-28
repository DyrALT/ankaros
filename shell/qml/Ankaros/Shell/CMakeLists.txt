if(ANKAROS_SHELL_SHARED)
    qt_add_library(ankaros_shell SHARED)
else()
    qt_add_library(ankaros_shell STATIC)
endif()

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

if(ANKAROS_SHELL_SHARED)
    # Qt'nin QML klasörü (Buildroot'ta /usr/qml). Modülü kullanan uygulamalar onu buradan bulur.
    set(module_dir ${QT6_INSTALL_QML}/Ankaros/Shell)
    install(TARGETS ankaros_shell LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
    install(TARGETS ankaros_shellplugin LIBRARY DESTINATION ${module_dir})
    install(FILES
        ${QT_QML_OUTPUT_DIRECTORY}/Ankaros/Shell/qmldir
        ${QT_QML_OUTPUT_DIRECTORY}/Ankaros/Shell/ankaros_shell.qmltypes
        DESTINATION ${module_dir}
    )
endif()
