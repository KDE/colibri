set(kcm_colibri_SRCS
    alignmentselector.cpp
    controlmodule.cpp
)

kde4_add_ui_files(kcm_colibri_SRCS
    controlmodule.ui
)

kde4_add_kcfg_files(kcm_colibri_SRCS
    ../app/config.kcfgc
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../app
)

kde4_add_plugin(kcm_colibri ${kcm_colibri_SRCS})
target_link_libraries(kcm_colibri
    ${KDE4_KDEUI_LIBS}
    ${KDE4_PLASMA_LIBS}
)

install(TARGETS kcm_colibri
    DESTINATION ${PLUGIN_INSTALL_DIR}
)

install(FILES colibri.desktop
    DESTINATION ${SERVICES_INSTALL_DIR}
)
