project(FractalUtils LANGUAGES CXX)
cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install)

if(NOT DEFINED CMAKE_INSTALL_PREFIX)
    set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install)
endif()

include(add_compiler_path_to_prefix.cmake)

add_subdirectory(core_utils)
add_subdirectory(png_utils)

install(
    EXPORT fractal_utils_targets
    FILE fractal_utils-config.cmake
    NAMESPACE fractal_utils::
    DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cmake/fractal_utils
)