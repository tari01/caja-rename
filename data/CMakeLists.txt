# libcaja-rename.caja-extension

find_package (Intltool REQUIRED)
string (TIMESTAMP YEAR "%Y")
configure_file ("${CMAKE_CURRENT_SOURCE_DIR}/libcaja-rename.caja-extension.in" "${CMAKE_CURRENT_BINARY_DIR}/libcaja-rename.caja-extension.intl" @ONLY)
intltool_merge_translations ("libcaja-rename.caja-extension.intl" "libcaja-rename.caja-extension" ALL UTF8)
install (FILES "${CMAKE_CURRENT_BINARY_DIR}/libcaja-rename.caja-extension" DESTINATION "${CMAKE_INSTALL_FULL_DATADIR}/caja/extensions")

# icons

install (DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/icons" DESTINATION "${CMAKE_INSTALL_FULL_DATADIR}")

# caja-rename-gresource.c

find_program (GLIB_COMPILE_RESOURCES NAMES glib-compile-resources REQUIRED)

add_custom_command (
    OUTPUT "${CMAKE_PROJECT_NAME}-gresource.c"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMAND "${GLIB_COMPILE_RESOURCES}"
    ARGS --target "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}-gresource.c" --generate-source "${CMAKE_PROJECT_NAME}.xml"
    VERBATIM
    MAIN_DEPENDENCY "${CMAKE_PROJECT_NAME}.xml"
    DEPENDS "${CMAKE_PROJECT_NAME}.ui"
)

add_custom_target ("data" DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}-gresource.c")
