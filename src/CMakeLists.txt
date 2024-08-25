# libcaja-rename.so

set_source_files_properties ("${CMAKE_BINARY_DIR}/data/${CMAKE_PROJECT_NAME}-gresource.c" PROPERTIES GENERATED TRUE)
add_library ("caja-rename" SHARED "rename.c" "caja-rename.c" "${CMAKE_BINARY_DIR}/data/${CMAKE_PROJECT_NAME}-gresource.c")
add_dependencies ("caja-rename" "data")
target_include_directories ("caja-rename" PUBLIC ${DEPS_INCLUDE_DIRS})
target_compile_options ("caja-rename" PUBLIC ${DEPS_CFLAGS_OTHER})
target_compile_definitions ("caja-rename" PUBLIC G_LOG_DOMAIN="caja-rename" LOCALEDIR="${CMAKE_INSTALL_FULL_LOCALEDIR}" GETTEXT_PACKAGE="${CMAKE_PROJECT_NAME}")
target_link_libraries ("caja-rename" ${DEPS_LIBRARIES})
target_link_options ("caja-rename" PUBLIC ${DEPS_LDFLAGS_OTHER})
target_link_directories ("caja-rename" PUBLIC ${DEPS_LIBRARY_DIRS})
install (TARGETS "caja-rename" DESTINATION "${EXTENSION_DIR}")
