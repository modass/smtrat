include(${carl_INCLUDE_DIR}/../cmake/carlmacros.cmake)

add_definitions(-fPIC)

configure_everything()

file(GLOB_RECURSE smtrat-common_sources "*.cpp")

add_new_libraries(smtrat-common ${PROJECT_VERSION_LIB} "${smtrat-common_sources}" "resources")
target_include_dirs_from(smtrat-common-objects "" PUBLIC carl-shared carl-settings-shared)
target_link_libraries(smtrat-common-shared carl-shared carl-settings-shared)
target_link_libraries(smtrat-common-static carl-static carl-settings-static)

install_libraries(smtrat smtrat-common-shared smtrat-common-static)
