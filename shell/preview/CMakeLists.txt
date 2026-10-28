# Kabuğu sıradan bir pencerede gösteren geliştirme uygulaması (macOS ve masaüstü Linux).
qt_add_executable(ankaros-shell-preview MACOSX_BUNDLE main.cpp)

qt_add_qml_module(ankaros-shell-preview
    URI Ankaros.Shell.Preview
    VERSION 1.0
    QML_FILES Main.qml
)

target_link_libraries(ankaros-shell-preview PRIVATE Qt6::Quick)
if(NOT ANKAROS_SHELL_SHARED)
    # Statik modülün eklentisi uygulamaya bağlanır ve main.cpp'de elle kaydedilir.
    target_link_libraries(ankaros-shell-preview PRIVATE ankaros_shellplugin ankaros_uiplugin ankaros_systemplugin)
    target_compile_definitions(ankaros-shell-preview PRIVATE ANKAROS_SHELL_STATIC)
endif()

install(TARGETS ankaros-shell-preview
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
