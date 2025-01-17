cmake_minimum_required(VERSION 3.17)
project(test_lasrsc)

if (WIN32)
    set(LIB_NAME laz_rs_c)
else ()
    set(LIB_NAME liblaz_rs_c)
endif ()

if (DEFINED ENV{CARGO_TARGET_DIR})
    set(CARGO_TARGET_DIR "$ENV{CARGO_TARGET_DIR}")
else ()
    set(CARGO_TARGET_DIR "${CMAKE_CURRENT_LIST_DIR}/target")
endif ()
# custom_target's COMMAND doesn't work if I give him a string
# so i don't know how to factorize this
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CARGO_TARGET "${CARGO_TARGET_DIR}/debug/${LIB_NAME}")
    add_custom_target(
            cargo_build
            ALL
            COMMAND cargo build
            WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
            VERBATIM
    )
else ()
    set(CARGO_TARGET "${CARGO_TARGET_DIR}/release/${LIB_NAME}")
    add_custom_target(
            cargo_build
            ALL
            COMMAND cargo build --release
            WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
            VERBATIM
    )
endif ()

add_library(lazrs SHARED IMPORTED)
add_dependencies(lazrs cargo_build)
target_include_directories(lazrs INTERFACE "${CMAKE_CURRENT_LIST_DIR}/include")
if (WIN32)
    set(CARGO_LIB "${CARGO_TARGET}.dll.lib")
    set_target_properties(lazrs PROPERTIES IMPORTED_LOCATION "${CARGO_TARGET}.dll")
    set_target_properties(lazrs PROPERTIES IMPORTED_IMPLIB "${CARGO_LIB}")
else ()
    set(CARGO_LIB "${CARGO_TARGET}.so")
    set_target_properties(lazrs PROPERTIES IMPORTED_LOCATION "${CARGO_LIB}")
endif ()


add_executable(test_lazrsc test_lazrsc.c)
target_link_libraries(test_lazrsc lazrs)

add_executable(test_compressor test_compressor.c)
target_link_libraries(test_compressor lazrs)

if (WIN32)
    add_custom_command(
            TARGET test_lazrsc POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy "${CARGO_TARGET}.dll" "$<TARGET_FILE_DIR:test_lazrsc>"
    )
    add_custom_command(
            TARGET test_compressor POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy "${CARGO_TARGET}.dll" "$<TARGET_FILE_DIR:test_compressor>"
    )
endif ()
