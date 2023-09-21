# libcaja-rename.caja-extension

string (TIMESTAMP YEAR "%Y")
configure_file ("${CMAKE_CURRENT_SOURCE_DIR}/libcaja-rename.caja-extension.in" "${CMAKE_CURRENT_BINARY_DIR}/libcaja-rename.caja-extension" @ONLY)
install (FILES "${CMAKE_CURRENT_BINARY_DIR}/libcaja-rename.caja-extension" DESTINATION "${CMAKE_INSTALL_FULL_DATADIR}/caja/extensions")

# caja-rename.svg

install (DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/icons/" DESTINATION "${CMAKE_INSTALL_FULL_DATADIR}/icons/hicolor")

# caja-rename-gresource.c

find_program (GLIB_COMPILE_RESOURCES NAMES glib-compile-resources REQUIRED)

add_custom_command (
    OUTPUT "caja-rename-gresource.c"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMAND "${GLIB_COMPILE_RESOURCES}"
    ARGS --target "${CMAKE_CURRENT_BINARY_DIR}/caja-rename-gresource.c" --generate-source "caja-rename.xml"
    VERBATIM
    MAIN_DEPENDENCY "caja-rename.xml"
    DEPENDS "caja-rename.ui"
)

add_custom_target ("data" DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/caja-rename-gresource.c")
