# Kabuğu sıradan bir pencerede gösteren geliştirme uygulaması (macOS ve masaüstü Linux).
qt_add_executable(ankaros-shell-preview MACOSX_BUNDLE main.cpp)

qt_add_qml_module(ankaros-shell-preview
    URI Ankaros.Shell.Preview
    VERSION 1.0
    QML_FILES Main.qml
)

target_link_libraries(ankaros-shell-preview PRIVATE Qt6::Quick ankaros_shellplugin)
