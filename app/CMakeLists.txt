# for dbus_add_activation_service
include(PkgConfigGetVar)

set(colibri_SRCS
    label.cpp
    main.cpp
    notificationmanager.cpp
    notificationwidget.cpp
)

qt4_add_dbus_adaptor(colibri_SRCS org.freedesktop.Notifications.xml
    notificationmanager.h Colibri::NotificationManager)

kde4_add_kcfg_files(colibri_SRCS
    config.kcfgc
)

kde4_add_executable(colibri ${colibri_SRCS})

target_link_libraries(colibri
    ${KDE4_KDEUI_LIBS}
    ${KDE4_PLASMA_LIBS}
    ${X11_Xext_LIB}
    ${X11_X11_LIB}
)

install(TARGETS colibri ${INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES colibri_autostart.desktop
    DESTINATION ${AUTOSTART_INSTALL_DIR}
)
