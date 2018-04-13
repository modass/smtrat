ExternalProject_Add(
	CArL-EP
	GIT_REPOSITORY "https://github.com/smtrat/carl.git"
		CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> -DBUILD_STATIC=ON
		BUILD_COMMAND ${CMAKE_MAKE_PROGRAM} lib_carl lib_carl_static
)

ExternalProject_Get_Property(CArL-EP INSTALL_DIR)

add_imported_library(CArL SHARED "${INSTALL_DIR}/lib/libcarl${DYNAMIC_EXT}" "${INSTALL_DIR}/include")
add_imported_library(CArL STATIC "${INSTALL_DIR}/lib/libcarl${STATIC_EXT}" "${INSTALL_DIR}/include")

add_library(lib_carl INTERFACE IMPORTED GLOBAL)
add_dependencies(lib_carl CArL_SHARED)
add_library(lib_carl_static INTERFACE IMPORTED GLOBAL)
add_dependencies(lib_carl CArL_STATIC)

add_dependencies(CArL_SHARED CArL-EP)
add_dependencies(CArL_STATIC CArL-EP)
add_dependencies(resources CArL_SHARED CArL_STATIC)
