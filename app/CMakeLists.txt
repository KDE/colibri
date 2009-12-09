# for dbus_add_activation_service
include(PkgConfigGetVar)


set(colibri_SRCS
    main.cpp
    notificationmanager.cpp
    notificationwidget.cpp
)

dbus_add_activation_service(org.freedesktop.Notifications.service.in)

qt4_add_dbus_adaptor(colibri_SRCS org.freedesktop.Notifications.xml
    notificationmanager.h Colibri::NotificationManager)

kde4_add_executable(colibri ${colibri_SRCS})

target_link_libraries(colibri
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KPARTS_LIBS}
    ${KDE4_PLASMA_LIBS}
)

install(TARGETS colibri ${INSTALL_TARGETS_DEFAULT_ARGS})
