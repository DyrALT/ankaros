# ankaros sistem servisleri: init (PID 1), oturum yöneticisi, ağ servisi, ankarosctl ve ortak kod.
# Qt kullanılmaz, açılışta Qt'nin yüklenmesi beklenmez (bkz. docs/kararlar/0009-kendi-init-sistemimiz.md).
# Birim testleri Mac'te de derlenir, servisler ve ankarosctl yalnızca Linux'ta.
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

add_library(ankaros-session-core STATIC
    session/accounts.cpp
    session/sessionconfig.cpp
)
target_include_directories(ankaros-session-core PUBLIC session)
target_link_libraries(ankaros-session-core PUBLIC ankaros-common)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    # crypt(3): glibc'den ayrılan libxcrypt
    target_link_libraries(ankaros-session-core PUBLIC crypt)
endif()

add_library(ankaros-network-core STATIC network/netconfig.cpp)
target_include_directories(ankaros-network-core PUBLIC network)
target_link_libraries(ankaros-network-core PUBLIC ankaros-common)

option(ANKAROS_SYSTEM_TESTS "Birim testlerini derle" ON)
if(ANKAROS_SYSTEM_TESTS)
    enable_testing()
    foreach(test config_test service_test session_test network_test)
        add_executable(${test} tests/${test}.cpp)
        target_link_libraries(${test} PRIVATE ankaros-services ankaros-session-core ankaros-network-core)
        add_test(NAME ${test} COMMAND ${test})
    endforeach()
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    # Unix soketleri (SOCK_CLOEXEC, SO_PEERCRED) Linux'a özgü
    target_sources(ankaros-common PRIVATE common/control.cpp common/protocol.cpp)

    add_executable(ankaros-init
        init/main.cpp
        init/log.cpp
        init/supervisor.cpp
        init/system.cpp
    )
    target_link_libraries(ankaros-init PRIVATE ankaros-services)

    add_executable(ankaros-session session/main.cpp)
    target_link_libraries(ankaros-session PRIVATE ankaros-session-core)

    add_executable(ankaros-network network/main.cpp)
    target_link_libraries(ankaros-network PRIVATE ankaros-network-core)

    add_executable(ankarosctl ctl/main.cpp)
    target_link_libraries(ankarosctl PRIVATE ankaros-common)

    install(TARGETS ankaros-init ankaros-session ankaros-network RUNTIME DESTINATION ${CMAKE_INSTALL_SBINDIR})
    install(TARGETS ankarosctl RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()
