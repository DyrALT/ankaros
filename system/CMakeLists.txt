# ankaros sistem servisleri: init (PID 1), ankarosctl ve ortak kod.
# Qt kullanılmaz, açılışta Qt'nin yüklenmesi beklenmez (bkz. docs/kararlar/0009-kendi-init-sistemimiz.md).
# Birim testleri Mac'te de derlenir, init ve ankarosctl yalnızca Linux'ta.
cmake_minimum_required(VERSION 3.21)
project(ankaros-system VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
include(GNUInstallDirs)

# PID 1'de istisna fırlatılmaz: hata dönüş değeriyle bildirilir.
add_compile_options(-Wall -Wextra -fno-exceptions)

add_library(ankaros-common STATIC
    common/command.cpp
    common/config.cpp
)
target_include_directories(ankaros-common PUBLIC common)

add_library(ankaros-services STATIC
    init/graph.cpp
    init/service.cpp
)
target_include_directories(ankaros-services PUBLIC init)
target_link_libraries(ankaros-services PUBLIC ankaros-common)

option(ANKAROS_SYSTEM_TESTS "Birim testlerini derle" ON)
if(ANKAROS_SYSTEM_TESTS)
    enable_testing()
    foreach(test config_test service_test)
        add_executable(${test} tests/${test}.cpp)
        target_link_libraries(${test} PRIVATE ankaros-services)
        add_test(NAME ${test} COMMAND ${test})
    endforeach()
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_executable(ankaros-init
        init/main.cpp
        init/control.cpp
        init/log.cpp
        init/supervisor.cpp
        init/system.cpp
    )
    target_link_libraries(ankaros-init PRIVATE ankaros-services)

    add_executable(ankarosctl ctl/main.cpp)
    target_link_libraries(ankarosctl PRIVATE ankaros-common)

    install(TARGETS ankaros-init RUNTIME DESTINATION ${CMAKE_INSTALL_SBINDIR})
    install(TARGETS ankarosctl RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()
