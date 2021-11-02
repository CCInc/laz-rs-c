cmake_minimum_required(VERSION 3.5)

project(laz-rs-c
        LANGUAGES CXX C
        VERSION 0.1.0)
set(CRATE_NAME ${PROJECT_NAME})

# build cargo crate
include(FetchContent)
FetchContent_Declare(
    Corrosion
    GIT_REPOSITORY https://github.com/AndrewGaspar/corrosion.git
    GIT_TAG f679545a63a8b214a415e086f910126ab66714fa
)
FetchContent_MakeAvailable(Corrosion)
corrosion_import_crate(MANIFEST_PATH "${CMAKE_SOURCE_DIR}/Cargo.toml"
    CRATES ${CRATE_NAME})
      
# includes
include(GNUInstallDirs)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

# Encourage user to specify a build type (e.g. Release, Debug, etc.), otherwise set it to Release.
if(NOT CMAKE_CONFIGURATION_TYPES)
    if(NOT CMAKE_BUILD_TYPE)
        message(STATUS "Setting build type to 'Release' as none was specified.")
        set_property(CACHE CMAKE_BUILD_TYPE PROPERTY VALUE "Release")
    endif()
endif()

# add headers to cargo lib
set(HEADERS include/lazrs/lazrs.h include/lazrs/las.h include/lazrs/point.h)
set_property(TARGET ${CRATE_NAME} PROPERTY PUBLIC_HEADER ${HEADERS})
target_include_directories(${CRATE_NAME} INTERFACE  
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/lazrs>  
    $<INSTALL_INTERFACE:include/lazrs>
)


# install targets
install(TARGETS ${CRATE_NAME}
    EXPORT laz-rs-c 
    LIBRARY DESTINATION lib 
    PUBLIC_HEADER DESTINATION include/lazrs)

# install the generated library files 
get_property(isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(${isMultiConfig})
    FOREACH(config ${CMAKE_CONFIGURATION_TYPES})
        string(TOUPPER ${config} CONFIG_UPPER)
        get_target_property(LIB_PATH ${CRATE_NAME}-shared IMPORTED_LOCATION_${CONFIG_UPPER})
        install(FILES ${LIB_PATH} DESTINATION lib OPTIONAL)

        if(WIN32)
            get_target_property(LIB_PATH ${CRATE_NAME}-shared IMPORTED_IMPLIB_${CONFIG_UPPER})
            install(FILES ${LIB_PATH} DESTINATION lib OPTIONAL)
        endif()
    ENDFOREACH(config)
else()
    string(TOUPPER ${CMAKE_BUILD_TYPE} CONFIG_UPPER)
    get_target_property(LIB_PATH ${CRATE_NAME}-shared IMPORTED_LOCATION_${CONFIG_UPPER})
    install(FILES ${LIB_PATH} DESTINATION lib)

    if(WIN32)
        get_target_property(LIB_PATH ${CRATE_NAME}-shared IMPORTED_IMPLIB_${CONFIG_UPPER})
        install(FILES ${LIB_PATH} DESTINATION lib)
    endif()
endif()

include(InstallBasicPackageFiles)
install_basic_package_files(${PROJECT_NAME}
                            NAMESPACE lazrs
                            VERSION ${${PROJECT_NAME}_VERSION}
                            COMPATIBILITY AnyNewerVersion)

# build example files
add_executable(test_lazrsc examples/test_lazrsc.cpp)
target_link_libraries(test_lazrsc ${CRATE_NAME})

add_executable(test_compressor examples/test_compressor.cpp)
target_link_libraries(test_compressor ${CRATE_NAME})