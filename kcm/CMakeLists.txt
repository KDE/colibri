set(kcm_colibri_SRCS
    colibrimodule.cpp
)

kde4_add_plugin(kcm_colibri ${kcm_colibri_SRCS})
target_link_libraries(kcm_colibri
    ${KDE4_KDEUI_LIBS}
)

install(TARGETS kcm_colibri
    DESTINATION ${PLUGIN_INSTALL_DIR}
)

install(FILES colibri.desktop
    DESTINATION ${SERVICES_INSTALL_DIR}
)
