# Ankaros.System: sistem servislerinin istemcileri (karar 0013). Session (ankaros-session), Network
# (ankaros-network), AppCatalog (uygulama listesi), SystemInfo (Ayarlar › Hakkında), Battery (alt çubuktaki pil). Compositor, kabuk ve uygulamalar bunları kullanır.
# Linux'ta C++ sınıfları servislerin Unix soketlerine bağlanır. Mac önizlemesinde aynı adlı sahte QML
# nesneleri (Session.qml, Network.qml) kullanılır, böylece arayüz servisler olmadan da denenebilir.
if(ANKAROS_SHELL_SHARED)
    qt_add_library(ankaros_system SHARED)
else()
    qt_add_library(ankaros_system STATIC)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    qt_add_qml_module(ankaros_system
        URI Ankaros.System
        VERSION 1.0
        SOURCES
            appcatalog.h
            appcatalog.cpp
            network.h
            network.cpp
            session.h
            session.cpp
            socketrequest.h
            socketrequest.cpp
            systeminfo.h
            systeminfo.cpp
            battery.h
            battery.cpp
    )
else()
    set_source_files_properties(Session.qml Network.qml SystemInfo.qml Battery.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)
    qt_add_qml_module(ankaros_system
        URI Ankaros.System
        VERSION 1.0
        QML_FILES
            Session.qml
            Network.qml
            SystemInfo.qml
            Battery.qml
        SOURCES
            appcatalog.h
            appcatalog.cpp
    )
endif()

# Uygulama listesi /etc/ankaros/apps.json'dan okunur, dosya yoksa (Mac önizlemesi) modüle gömülü olandan
qt_add_resources(ankaros_system "apps"
    PREFIX "/qt/qml/Ankaros/System"
    FILES apps.json
)

target_link_libraries(ankaros_system PRIVATE Qt6::Quick)
ankaros_install_qml_module(ankaros_system Ankaros/System)
