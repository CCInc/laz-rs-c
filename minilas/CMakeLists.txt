add_library(minilas)

target_include_directories(minilas PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

target_sources(minilas PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/src/las.c"
        "${CMAKE_CURRENT_LIST_DIR}/include/minilas/las.h"
        )

install(TARGETS minilas EXPORT minilas-targets
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION lib)
install(EXPORT minilas-targets DESTINATION lib/cmake/minilas)
install(DIRECTORY include/minilas DESTINATION include)