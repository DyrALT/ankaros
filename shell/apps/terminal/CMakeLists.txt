# Terminal uygulaması (karar 0014). Ekran durumunu libvterm tutar, metni TerminalView çizer.
find_package(PkgConfig REQUIRED)
pkg_check_modules(VTERM REQUIRED IMPORTED_TARGET vterm)

# Ekran tamponu ayrı bir kütüphanede, birim testi de kullanır
add_library(ankaros_terminal_screen STATIC terminalscreen.h terminalscreen.cpp)
target_link_libraries(ankaros_terminal_screen PUBLIC Qt6::Gui PkgConfig::VTERM)

qt_add_executable(ankaros-terminal MACOSX_BUNDLE main.cpp)

qt_add_qml_module(ankaros-terminal
    URI Ankaros.Apps.Terminal
    VERSION 1.0
    QML_FILES
        Main.qml
    SOURCES
        terminalview.h
        terminalview.cpp
        ptyprocess.h
        ptyprocess.cpp
    RESOURCES
        fonts/JetBrainsMono-Regular.ttf
        fonts/JetBrainsMono-Bold.ttf
        fonts/JetBrainsMono-LICENSE.txt
)

target_link_libraries(ankaros-terminal PRIVATE Qt6::Quick ankaros_terminal_screen)
if(NOT APPLE)
    target_link_libraries(ankaros-terminal PRIVATE util)
endif()
if(NOT ANKAROS_SHELL_SHARED)
    target_link_libraries(ankaros-terminal PRIVATE ankaros_uiplugin)
    target_compile_definitions(ankaros-terminal PRIVATE ANKAROS_SHELL_STATIC)
endif()

install(TARGETS ankaros-terminal
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(ANKAROS_SHELL_TESTS)
    find_package(Qt6 REQUIRED COMPONENTS Test)
    qt_add_executable(terminalscreen_test tests/terminalscreen_test.cpp)
    target_link_libraries(terminalscreen_test PRIVATE ankaros_terminal_screen Qt6::Test)
    add_test(NAME terminalscreen_test COMMAND terminalscreen_test)

    qt_add_executable(ptyprocess_test tests/ptyprocess_test.cpp ptyprocess.h ptyprocess.cpp)
    target_link_libraries(ptyprocess_test PRIVATE Qt6::Core Qt6::Test)
    if(NOT APPLE)
        target_link_libraries(ptyprocess_test PRIVATE util)
    endif()
    add_test(NAME ptyprocess_test COMMAND ptyprocess_test)
endif()
