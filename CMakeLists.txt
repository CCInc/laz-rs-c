cmake_minimum_required(VERSION 3.17)
project(laz-rs-c)

add_subdirectory(lazrs)
add_subdirectory(cpp)
add_subdirectory(minilas)
add_subdirectory(examples)

install(TARGETS lazrs_cpp laz-rs-c minilas EXPORT lazrs-targets
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION lib)
install(EXPORT lazrs-targets DESTINATION lib/cmake/lazrs NAMESPACE lazrs::)