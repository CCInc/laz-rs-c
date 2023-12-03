add_library(lazrs_cpp SHARED src/lazrs_cpp.cpp)

target_include_directories(lazrs_cpp PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

target_link_libraries(lazrs_cpp PUBLIC laz-rs-c)
install(DIRECTORY include/lazrs DESTINATION include)